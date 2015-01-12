#pragma once

#include "phasor-lua.hpp"
#include "../Common/Streams.h"
#include <unordered_set>
#include <memory>
#include <list>

namespace scripting {

    template <typename... ResultTypes> class Caller;

    void checkEvents();

    class PhasorScript : public std::enable_shared_from_this<PhasorScript> {
    private:
        lua::State state;
        std::string file, name;
        bool persistent;
        bool active; // false if OnScriptUnload has been called
        std::unordered_set<std::string> blockedFunctions;

        static const char thisKey;
        
        PhasorScript(bool persistent, std::string file, std::string name)
            : file(std::move(file)), name(std::move(name)), persistent(persistent), active(true)
        {}

        template <class Itr>
        void load(Itr itr, const Itr end) {
            // store reference to this so we can access it from script callbacks..
            lua_pushlightuserdata(state, (void *)&thisKey); //key
            lua_pushlightuserdata(state, this); // value
            lua_settable(state, LUA_REGISTRYINDEX);

            for (; itr != end; ++itr)
                state.registerFunction(itr->cfunc.name, itr->cfunc.func);
           
            // For better error logs
            state.doString(R"(
local __f_stacktrace = loadfile("lua\\StackTracePlus.lua")
if __f_stacktrace then
STP = __f_stacktrace()
end
                )");
            // Print causes issues once the console has initialized
            state.doString("print = hprintf");
            state.doFile(this->file);
        }

    public:

        template <class Itr>
        static std::shared_ptr<PhasorScript> create(bool persistent, std::string file, std::string name, Itr itr, const Itr end) {
            // Some API functions need a valid shared_ptr before they will work, so split the construction
            std::shared_ptr<PhasorScript> p(new PhasorScript(persistent, std::move(file), std::move(name)));
            p->load(itr, end);
            return p;
        }

        static PhasorScript& get(lua_State* L);

        inline std::shared_ptr<PhasorScript> get() { return shared_from_this(); }
        inline lua::State& getState() { return state; }
        inline const lua::State& getState() const { return state; }
        inline const std::string& getName() const { return name; }
        inline bool isPersistent() const { return persistent; }
        inline bool isActive() const { return active;  }
        void inactive();

        void block(std::string f);
        bool isBlocked(const std::string& f) const;
    };

    class ScriptHandler {
    private:
        std::vector<std::shared_ptr<PhasorScript>> scripts;
        COutStream& errStream;
        std::string scriptDir;

        void unloadScript(PhasorScript& script);
        bool loadScript(const std::string& script, bool persistent, std::shared_ptr<PhasorScript>& out);

    public:
        ScriptHandler(std::string scriptDir, COutStream& errStream);

        bool isLoaded(const std::string& script);

        bool loadScript(const std::string& script, bool persistent);
        void loadPersistentScripts();
        bool unloadScript(const std::string& script);
        void unloadAllScripts(bool includePersistent);
        bool reloadScript(const std::string& script);
        void reloadAllScripts(bool includePersistent);

        std::vector<std::pair<std::string, bool>> getLoadedScripts() const;

        void handleError(PhasorScript& script, const std::string& func, const char* what);

        template <typename... ResultTypes> friend class Caller;
    };

    template <typename... ResultTypes>
    class Caller {

    public:
        template <typename... ArgTypes>
        static inline boost::optional<std::tuple<ResultTypes...>>
            call(ScriptHandler& h, const std::string& func, std::tuple<ArgTypes...>& uargs)
        {
            return call(h, func, true, std::move(uargs));
        }

        template <typename... ArgTypes>
        static inline boost::optional<std::tuple<ResultTypes...>>
            call(ScriptHandler& h, const std::string& func, bool relevant, std::tuple<ArgTypes...>& uargs)
        {
            boost::optional<std::tuple<ResultTypes...>> result;
            std::tuple<ArgTypes..., bool> args = std::tuple_cat(std::move(uargs), std::make_tuple(relevant));
            const size_t lastElem = std::tuple_size<decltype(args)>::value-1;

            for (auto itr = h.scripts.begin(); itr != h.scripts.end(); ++itr) {
                auto x = call_single(h, **itr, func.c_str(), args);
                if (x && !result) { // got all return values successfully
                    result = x;
                    std::get<lastElem>(args) = false;
                }
            }

            return result;
        }

        template <typename... ArgTypes>
        static inline boost::optional<std::tuple<ResultTypes...>>
            call_single(ScriptHandler& h, PhasorScript& script, const std::string& func, std::tuple<ArgTypes...>& args)
        {
            boost::optional<std::tuple<ResultTypes...>> result;
            if (!script.isBlocked(func)) {
                phlua::Caller<ResultTypes...> c(script.getState());

                try {
                    auto x = c.call(func.c_str(), args);
                    if (!c.hasError()) { // got all return values successfully
                        result = x;
                    }
                } catch (std::exception& e) {
                    h.handleError(script, func, e.what());
                }
            }
            return result;
        }
    };

}

extern std::unique_ptr<scripting::ScriptHandler> g_Scripts;