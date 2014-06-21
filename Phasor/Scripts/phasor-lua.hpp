#pragma once

#include "lua-bindings.hpp"
#include "../Phasor/Halo/Halo.h"
#include "../Phasor/Halo/Player.h"
#include "../Phasor/Halo/Game/Objects.h"
#include "../Phasor/Halo/Game/Game.h"
#include "../Phasor/Halo/tags.h"
#include "../Common/vect3d.h"

namespace phlua {

	struct PhasorPush : public lua::Push {

		PhasorPush(lua_State* L)
			: Push(L)
		{}

		using lua::Push::operator();

		void operator()(const halo::ident& x) {
			operator()((unsigned long)x);
		}

		void operator()(const vect3d& v) {
			operator()(v.x);
			operator()(v.y);
			operator()(v.z);
		}

		void operator()(const halo::s_tag_entry* x) {
			operator()((unsigned long)x);
		}

		void operator()(const halo::objects::s_halo_object* x) {
			operator()((unsigned long)x);
		}

		template <typename T>
		void operator()(const boost::optional<T>& x) {
			if (!x) operator()(lua::types::Nil());
			else operator()(*x);
		}
	};

	struct PhasorPop : public lua::Pop {
	private:

		void raise_error(int n, const char* msg) {
			if (mode == e_mode::kRaiseError) {
				luaL_argerror(L, n, msg);
			} else {
				err = true;
			}
		}

	public:

		/* msvc needs inheriting constructors! */
		PhasorPop(lua_State* L, int n, e_mode mode)
			: Pop(L, n, mode)
		{}

		using lua::Pop::operator();

		void operator()(halo::ident& x) {
			unsigned long id;
			operator()(id);
			x = halo::make_ident(id);
		}

		void operator()(halo::s_tag_entry*& tag) {
			halo::ident id;
			operator()(id);
			halo::s_tag_entry* t = halo::LookupTag(id);
			if (!t) raise_error(n-1, "invalid tag id");
			else tag = t;
		}

		void operator()(vect3d& x) {
			vect3d tmp;
			operator()(tmp.x);
			operator()(tmp.y);
			operator()(tmp.z);
			x = tmp;
		}

		// --------------------------------------------------------------

		void operator()(halo::s_player*& player) {
			int id;
			operator()(id);
			halo::s_player* p = halo::game::getPlayer(id);
			if (!p) raise_error(n-1, "invalid player id");
			else player = p;
		}

		void operator()(boost::optional<halo::s_player*>& player) {
			boost::optional<int> id;
			operator()(id);
			halo::s_player* p = halo::game::getPlayer(*id);
			if (p) player = p;
		}

		// --------------------------------------------------------------

		void operator()(halo::objects::s_halo_object*& object) {
			halo::ident id;
			operator()(id);
			halo::objects::s_halo_object* obj = static_cast<halo::objects::s_halo_object*>(halo::objects::GetObjectAddress(id));
			if (!obj) raise_error(n-1, "invalid object id");
			else object = obj;
		}

		void operator()(boost::optional<halo::objects::s_halo_object*>& object) {
			boost::optional<halo::ident> id;
			operator()(id);
			halo::objects::s_halo_object* obj = static_cast<halo::objects::s_halo_object*>(halo::objects::GetObjectAddress(*id));
			if (obj) object = obj;
		}

		// --------------------------------------------------------------

		template <typename Y>
		void operator()(boost::optional<Y>& x) {
			if (lua_type(L, -1) == LUA_TNIL) {
				pop(L);
			} else {
				Y val;
				operator()(val);
				x = val;
			}
		}
	};

	/* Just for convenience (don't need to specify PhasorPush/Pop */
	template <typename... ResultTypes>
	class Caller : public ::lua::Caller<ResultTypes...> {
	public:
		Caller(::lua::State& L)
			: ::lua::Caller(L)
		{}

		template <typename... ArgTypes>
		inline std::tuple<ResultTypes...> call(const std::string& func, 
			const std::tuple<ArgTypes...>& args)
		{
			return ::lua::Caller.call<PhasorPush, PhasorPop, ArgTypes...>(func, args);
		}
	};

	namespace callback {

		template <typename... ResultTypes>
		inline std::tuple<ResultTypes...> getArguments(lua_State* L, const char* funcname) {
			return ::lua::callback::getArguments<PhasorPop, ResultTypes...>(L, funcname);
		}

		template <typename... Types>
		inline int pushReturns(lua_State* L, const std::tuple<Types...>& t) {
			return ::lua::callback::pushReturns<PhasorPush, Types...>(L, t);
		}
	}
}