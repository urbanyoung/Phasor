#include "../phasor-lua.hpp"
#include "../../Common/Timers.h"
#include "../scripting.hpp"
#include "../../Phasor/Globals.h"
#include <memory>

class ScriptTimer : public TimerEvent {
private:
    std::weak_ptr<scripting::PhasorScript> state;
    std::string func;
    lua::types::AnyRef userdata;
    size_t count;    

public:
    ScriptTimer(std::weak_ptr<scripting::PhasorScript> state, 
                std::string func, lua::types::AnyRef userdata,
                size_t delay)
        : TimerEvent(delay),
        state(std::move(state)), func(std::move(func)), 
        userdata(std::move(userdata)), count(0)
    {}

    virtual bool OnExpiration(Timers&) override {
        auto s = state.lock();
        if (!s) return false;

        boost::optional<bool> reset;
        phlua::Caller<bool> c(s->getState());
        std::tie(reset) = c.call(func.c_str(), std::make_tuple(GetID(), ++count, std::cref(userdata)));
        if (!reset) *reset = false;

        return *reset;
    }
};

std::string makeRegistryKey(size_t id) {
    static const std::string prefix = "phasor_timer_";
    return prefix + std::to_string(id);
}

int l_registertimer(lua_State* L) {
    using namespace scripting;
    size_t delay;
    std::string callback;
    lua::types::AnyRef userdata;

    std::tie(delay, callback, userdata) = phlua::callback::getArguments<size_t, std::string, decltype(userdata)>(L, __FUNCTION__);

    PhasorScript& state = PhasorScript::get(L);
    std::shared_ptr<PhasorScript> pstate = state.shared_from_this();
    std::weak_ptr<PhasorScript> wp(pstate);

    timer_ptr timer(new ScriptTimer(std::move(wp), std::move(callback), std::move(userdata), delay));
    size_t id = g_Timers.AddTimer(std::move(timer));
    std::string registryKey = makeRegistryKey(id);

    // Stores this timer in the script
    lua_pushstring(L, registryKey.c_str());
    lua_pushnumber(L, id);
    lua_settable(L, LUA_REGISTRYINDEX);

    return phlua::callback::pushReturns(L, std::make_tuple(id));
}

int l_removetimer(lua_State* L) {
    size_t id;
    std::tie(id) = phlua::callback::getArguments<size_t>(L, __FUNCTION__);

    // See if it's a valid timer for this script..
    std::string registryKey = makeRegistryKey(id);
    lua_pushstring(L, registryKey.c_str());
    lua_gettable(L, LUA_REGISTRYINDEX);
    bool valid = lua_type(L, -1) != LUA_TNIL;
    lua_pop(L, 1);

    if (valid) {
        if (!g_Timers.RemoveTimer(id))
            luaL_error(L, "attempt to remove timer within its callback");

        // remove from registry
        lua_pushnil(L);
        lua_setfield(L, LUA_REGISTRYINDEX, registryKey.c_str());
    }

    return 0;
}