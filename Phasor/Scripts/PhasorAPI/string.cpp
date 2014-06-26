#include "string.h"
#include "../phasor-lua.hpp"

int l_tokenizestring(lua_State* L) {
    std::string str, delim;
    std::tie(str, delim) = phlua::callback::getArguments<std::string, std::string>(L, __FUNCTION__);

    auto tokens = Tokenize<std::string>(str, delim);
    return phlua::callback::pushReturns(L, std::make_tuple(std::cref(tokens)));
}

int l_tokenizecmdstring(lua_State* L) {
    std::string str;
    std::tie(str) = phlua::callback::getArguments<std::string>(L, __FUNCTION__);

    auto tokens = TokenizeArgs(str);
    return phlua::callback::pushReturns(L, std::make_tuple(std::cref(tokens)));
}