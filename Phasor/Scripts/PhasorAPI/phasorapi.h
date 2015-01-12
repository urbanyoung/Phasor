#pragma once

#include "../lua-bindings.hpp"
#include <vector>

namespace phasorapi {
    enum class AccessMode {
        kAlways, kWhileLoaded
    };

    struct Func {
        lua::CFunc cfunc;
        AccessMode access;
    };

    int l_func_inactive(lua_State* L);

    extern std::vector<Func> funcTable;
}