#include "phasorapi.h"

namespace phasorapi {
	int test(lua_State* L) {
		return 0;
	}

	std::vector<lua::callback::CFunc> funcTable{
		{"test", &test},
	};
}