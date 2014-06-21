#include "haloobjects.h"
#include "../phasor-lua.hpp"
#include "../../Phasor/Halo/Game/Objects.h"

int l_getobject(lua_State* L) {
	boost::optional<halo::objects::s_halo_object*> object;
	std::tie(object) = phlua::callback::getArguments<decltype(object)>(L, __FUNCTION__);
	return phlua::callback::pushReturns(L, std::make_tuple(object));
}

int l_getobjectcoords(lua_State* L) {
	halo::objects::s_halo_object* object;
	std::tie(object) = phlua::callback::getArguments<decltype(object)>(L, __FUNCTION__);
	return phlua::callback::pushReturns(L, std::make_tuple(std::ref(object->location)));
}

int l_objectaddrtoplayer(lua_State* L) {
	unsigned long addr;
	std::tie(addr) = phlua::callback::getArguments<unsigned long>(L, __FUNCTION__);
	
	boost::optional<int> playerId;
	for (int i = 0; i < 16; i++) {
		halo::s_player* player = halo::game::getPlayer(i);
		if (player && halo::objects::GetObjectAddress(player->mem->object_id) == (void*)addr) {
			playerId = i;
			break;
		}
	}

	return phlua::callback::pushReturns(L, std::make_tuple(playerId));
}

int l_objectidtoplayer(lua_State* L) {
	boost::optional<halo::ident> objId;
	std::tie(objId) = phlua::callback::getArguments<decltype(objId)>(L, __FUNCTION__);

	boost::optional<int> playerId;
	if (objId) {
		for (int i = 0; i < 16; i++) {
			halo::s_player* player = halo::game::getPlayer(i);
			if (player && player->mem->object_id == objId) {
				playerId = i;
				break;
			}
		}
	}

	return phlua::callback::pushReturns(L, std::make_tuple(playerId));
}

int l_createobject(lua_State* L) {
	halo::s_tag_entry* tag;
	halo::ident parentId;
	boost::optional<int> respawnTime;
	bool do_respawn;
	vect3d pos;

	std::tie(tag, parentId, respawnTime, do_respawn, pos) =
		phlua::callback::getArguments
		<
			decltype(tag),
			decltype(parentId),
			decltype(respawnTime),
			decltype(do_respawn),
			decltype(pos)
		>(L, __FUNCTION__);

	if (parentId == 0) parentId = halo::ident();

	halo::ident objid;
	bool success = halo::objects::CreateObject(tag, parentId, respawnTime, do_respawn, &pos, objid);

	if (success) {
		return phlua::callback::pushReturns(L, std::make_tuple(objid));
	} else {
		return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil()));
	}
}