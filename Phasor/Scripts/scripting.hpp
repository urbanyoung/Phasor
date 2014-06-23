#pragma once

#include "phasor-lua.hpp"
#include "../Common/Streams.h"
#include <unordered_set>
#include <memory>

namespace scripting {

	template <typename... ResultTypes> class Caller;

	class PhasorScript : public std::enable_shared_from_this<PhasorScript> {
	private:
		lua::State state;
		std::string file, name;
		bool persistent;
		std::unordered_set<std::string> blockedFunctions;
		static const char thisKey;

		template <class Itr>
		PhasorScript(bool persistent, std::string file, std::string name, Itr itr, const Itr end)
			: file(std::move(file)), name(std::move(name)), persistent(persistent)
		{
			// store reference to this so we can access it from script callbacks..
			lua_pushlightuserdata(state, (void *)&thisKey); //key
			lua_pushlightuserdata(state, this); // value
			lua_settable(state, LUA_REGISTRYINDEX);

			lua::callback::registerFunctions(state, itr, end);
			state.doFile(file);
		}

	public:
		
		template <class Itr>
		static std::shared_ptr<PhasorScript> create(bool persistent, std::string file, std::string name, Itr itr, const Itr end) {
			return std::shared_ptr<PhasorScript>(new PhasorScript(persistent, std::move(file), std::move(name), itr, end));
		}

		static PhasorScript& get(lua_State* L);
		
		inline std::shared_ptr<PhasorScript> get() { return shared_from_this(); }
		inline lua::State& getState() { return state;  }
		inline const lua::State& getState() const { return state; }
		inline const std::string& getName() const { return name;  }
		inline bool isPersistent() const { return persistent;  }

		void block(std::string f);
		bool isBlocked(const std::string& f) const;
	};

	class ScriptHandler {
	private:
		std::vector<std::shared_ptr<PhasorScript>> scripts;
		COutStream& errStream;
		std::string scriptDir;

		void handleError(PhasorScript& script, const std::string& func, const char* what);
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

		std::vector<std::string> getLoadedScripts() const;

		template <typename... ResultTypes> friend class Caller;
	};

	template <typename... ResultTypes>
	class Caller {
	public:
		template <typename... ArgTypes>
		static inline boost::optional<std::tuple<ResultTypes...>>
			call(ScriptHandler& h, const std::string& func, const std::tuple<ArgTypes...>& args)
		{
			boost::optional<std::tuple<ResultTypes...>> result;

			for (auto itr = h.scripts.begin(); itr != h.scripts.end(); ++itr) {
				PhasorScript& script = *itr;
				if (script.isBlocked(func)) continue;
				phlua::Caller<ResultTypes...> c<(script.getState());

				try {
					auto x = c.call(func.c_str(), args);
					if (!c.hasError() && !result) { // got all return values successfully
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