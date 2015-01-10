#include "playerinfo.h"
#include "../phasor-lua.hpp"
#include "../../Phasor/Halo/Player.h"
#include "../../Phasor/Halo/Game/Game.h"
#include "../../Phasor/Admin.h"

int l_resolveplayer(lua_State* L) {
    halo::s_player* player;
    std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
    return phlua::callback::pushReturns(L, std::make_tuple(player->mem->client_stuff.machineId+1));
}

int l_rresolveplayer(lua_State* L) {
    size_t machineId;
    std::tie(machineId) = phlua::callback::getArguments<size_t>(L, __FUNCTION__);
    // note the -1
    halo::s_player* player = halo::game::getPlayerFromRconId(machineId-1);

    if (player)
        return phlua::callback::pushReturns(L, std::make_tuple(player->memory_id));
    else
        return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil()));
}

int l_getplayer(lua_State* L) {
    boost::optional<halo::s_player*> player;
    std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);

    if (player)
        return phlua::callback::pushReturns(L, std::make_tuple((size_t)(*player)->mem));
    else
        return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil()));
}

int l_getip(lua_State* L) {
    halo::s_player* player;
    std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
    return phlua::callback::pushReturns(L, std::make_tuple(player->ip));
}

int l_getport(lua_State* L) {
    halo::s_player* player;
    std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
    return phlua::callback::pushReturns(L, std::make_tuple(player->port));
}

int l_getteam(lua_State* L) {
    halo::s_player* player;
    std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
    return phlua::callback::pushReturns(L, std::make_tuple(player->mem->team));
}

int l_getname(lua_State* L) {
    halo::s_player* player;
    std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
    return phlua::callback::pushReturns(L, std::make_tuple(player->mem->playerName));
}

int l_gethash(lua_State* L) {
    halo::s_player* player;
    std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
    return phlua::callback::pushReturns(L, std::make_tuple(std::cref(player->hash)));
}

int l_getteamsize(lua_State* L) {
    unsigned char team;
    std::tie(team) = phlua::callback::getArguments<size_t>(L, __FUNCTION__);

    unsigned char count = 0;
    for (int i = 0; i < 16; i++) {
        halo::s_player* player = halo::game::getPlayer(i);
        if (player && player->mem->team == team) count++;
    }
    return phlua::callback::pushReturns(L, std::make_tuple(count));
}

int l_getplayerobjectid(lua_State* L) {
    boost::optional<halo::s_player*> player;
    std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);

    if (player && (*player)->mem->object_id.valid())
        return phlua::callback::pushReturns(L, std::make_tuple((*player)->mem->object_id));
    else
        return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil()));
}

// Now returns both m_playerObj and playerObjId
int l_getplayerobject(lua_State* L) {
	boost::optional<halo::s_player*> player;
	std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);

	boost::optional<halo::ident> playerObjId;
	boost::optional<halo::objects::s_halo_object*> m_playerObj;
	if (player) {
		playerObjId = (*player)->mem->object_id;
		m_playerObj = (halo::objects::s_halo_object*)halo::objects::GetObjectAddress(*playerObjId);
	}
	if (m_playerObj && *m_playerObj != 0)
		return phlua::callback::pushReturns(L, std::make_tuple(m_playerObj, playerObjId));
	else
		return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil(), playerObjId));
}

int l_getplayerweaponid(lua_State* L) {
	boost::optional<halo::s_player*> player;
	boost::optional<int> slot;
	std::tie(player, slot) = phlua::callback::getArguments<decltype(player), int>(L, __FUNCTION__);

	if (player && (*player)->mem->object_id.valid())	{
		halo::objects::s_halo_object* m_playerObj = (halo::objects::s_halo_object*)halo::objects::GetObjectAddress((*player)->mem->object_id);
		// use vehicle weapon if player is in a vehicle and ignore slot argument entirely (vehicles can only have one weapon)
		if (m_playerObj->vehicleId.valid())
			return phlua::callback::pushReturns(L, std::make_tuple(((halo::objects::s_halo_unit*)(halo::objects::GetObjectAddress(m_playerObj->vehicleId)))->weaponObjId[0]));
		// if slot argument is valid use objId of weapon at specified slot
		else if (slot && slot >= 0 && slot <= 4)
			return phlua::callback::pushReturns(L, std::make_tuple(((halo::objects::s_halo_unit*)m_playerObj)->weaponObjId[*slot]));
		// otherwise use the current weapon the player is holding
		else
			return phlua::callback::pushReturns(L, std::make_tuple(m_playerObj->player_curWeapon));
	}
}

// Returns both m_weaponObj and weaponObjId
int l_getplayerweapon(lua_State* L) {
	boost::optional<halo::s_player*> player;
	boost::optional<int> slot;
	std::tie(player, slot) = phlua::callback::getArguments<decltype(player), decltype(slot)>(L, __FUNCTION__);

	boost::optional<halo::ident> weaponObjId;
	if (player && (*player)->mem->object_id.valid())	{
		halo::objects::s_halo_object* m_playerObj = (halo::objects::s_halo_object*)halo::objects::GetObjectAddress((*player)->mem->object_id);
		// use vehicle weapon if player is in a vehicle and ignore slot argument entirely (vehicles can only have one weapon)
		if (m_playerObj->vehicleId.valid())
			weaponObjId = ((halo::objects::s_halo_unit*)(halo::objects::GetObjectAddress(m_playerObj->vehicleId)))->weaponObjId[0];
		// if slot argument is valid use objId of weapon at specified slot
		else if (slot && slot >= 0 && slot <= 4)
			weaponObjId = ((halo::objects::s_halo_unit*)m_playerObj)->weaponObjId[*slot];
		// otherwise use the current weapon the player is holding
		else
			weaponObjId = m_playerObj->player_curWeapon;

		if (weaponObjId && (*weaponObjId).valid()) {
			halo::objects::s_halo_object* m_weaponObj = (halo::objects::s_halo_object*)halo::objects::GetObjectAddress(*weaponObjId);
			if (m_weaponObj != 0)
				return phlua::callback::pushReturns(L, std::make_tuple(m_weaponObj, *weaponObjId));
			else
				return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil(), *weaponObjId));
		}
		else if (weaponObjId)
			return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil(), *weaponObjId));
	}

	// player/playerObjId isn't valid or somehow weaponObjId isn't valid
	return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil(), lua::types::Nil()));

}

int l_getplayervehicleid(lua_State* L) {
	boost::optional<halo::s_player*> player;
	std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);

	if (player && (*player)->mem->object_id.valid())
		return phlua::callback::pushReturns(L, std::make_tuple(((halo::objects::s_halo_object*)halo::objects::GetObjectAddress((*player)->mem->object_id))->vehicleId));
	else
		return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil()));
}

// Returns both m_vehicleObj and vehicleObjId
int l_getplayervehicle(lua_State* L) {
	boost::optional<halo::s_player*> player;
	std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);

	boost::optional<halo::objects::s_halo_object*> object;
	if (player)	{
		object = (halo::objects::s_halo_object*)halo::objects::GetObjectAddress((*player)->mem->object_id);
		if (object && (*object)->vehicleId.valid())	{
			halo::objects::s_halo_object* m_vehicleObj = (halo::objects::s_halo_object*)halo::objects::GetObjectAddress((*object)->vehicleId);
			if (m_vehicleObj != 0)
				return phlua::callback::pushReturns(L, std::make_tuple(m_vehicleObj, (*object)->vehicleId));
			else
				return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil(), (*object)->vehicleId));
		}
		else if (object != 0)
			return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil(), (*object)->vehicleId));
	}
	return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil(), lua::types::Nil()));
}

int l_isadmin(lua_State* L) {
    halo::s_player* player;
    std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
    return phlua::callback::pushReturns(L, std::make_tuple(player->is_admin));
}

int l_getadminlvl(lua_State* L) {
    halo::s_player* player;
    std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);

    int level;
    if (Admin::getLevel(player->hash, &level)) {
        return phlua::callback::pushReturns(L, std::make_tuple(level));
    } else {
        return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil()));
    }
}

int l_setadmin(lua_State* L) {
    halo::s_player* player;
    std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
    player->is_admin = true;
    return 0;
}