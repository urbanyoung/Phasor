#pragma once

#include "lua-bindings.hpp"
#include "Phasor/Halo/Halo.h"
#include "Phasor/Halo/Player.h"

namespace lua {

	struct PhasorPop : public lua::Pop {

		using lua::Pop::operator();

		void operator()(halo::ident& x) {
			unsigned long id;
			operator()(id);
			x = halo::make_ident(id);
		}

		void operator()(halo::s_player** player) {
			int id;
			operator()(id);
			*player = halo::game::getPlayer(id);
			if (!*player) luaL_error(L, )
		}
	};

	struct PhasorPush : public lua::Push {

		using lua::Push::operator();

		void operator()(const halo::ident& x) {
			operator()((unsigned long)x);
		}
	};


}