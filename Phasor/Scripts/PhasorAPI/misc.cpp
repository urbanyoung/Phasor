#include "misc.h"
#include "../phasor-lua.hpp"
#include <windows.h>
#include <chrono>
#include <random>

auto seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 rng(static_cast<unsigned int>(seed));

int l_getticks(lua_State* L) {
    // just to ensure the lua stack doesn't have junk on it..
    phlua::callback::getArguments<>(L, __FUNCTION__);
    return phlua::callback::pushReturns(L, std::make_tuple(GetTickCount()));
}

int l_getrandomnumber(lua_State* L) {
    size_t min, max;
    std::tie(min, max) = phlua::callback::getArguments<size_t, size_t>(L, __FUNCTION__);

    size_t result = max;
    if (min != max)  {
        std::uniform_int_distribution<size_t> dist(min, max-1);
        result = dist(rng);
    }

    return phlua::callback::pushReturns(L, std::make_tuple(result));
}
