// lua.hpp
// Lua header files for C++
// We compile Lua as C++ but don't want its name mangling
extern "C" {
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
};
