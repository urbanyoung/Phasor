#include "playerinfo.h"
#include "api_readers.h"
#include "../Phasor/Admin.h"

using namespace Common;
using namespace Manager;

void l_resolveplayer(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	halo::s_player* player = ReadPlayer(handler, *args[0], true);
	AddResultNumber(player->mem->playerNum + 1, results);
}

void l_rresolveplayer(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	DWORD machine_id = ReadNumber<DWORD>(*args[0]) - 1;
	halo::s_player* player = halo::game::GetPlayerFromRconId(machine_id);
	if (!player) AddResultNil(results);
	else AddResultNumber(player->memory_id, results);
}

void l_getplayer(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	halo::s_player* player = ReadPlayer(handler, *args[0], false);
	if (!player) AddResultNil(results);
	else AddResultPtr(player->mem, results);
}

void l_getip(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	halo::s_player* player = ReadPlayer(handler, *args[0], true);
	AddResultString(player->ip, results);
}

void l_getport(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	halo::s_player* player = ReadPlayer(handler, *args[0], true);
	AddResultNumber(player->port, results);
}

void l_getteam(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	halo::s_player* player = ReadPlayer(handler, *args[0], true);
	AddResultNumber(player->mem->team, results);
}

void l_getname(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	halo::s_player* player = ReadPlayer(handler, *args[0], true);
	AddResultString(player->mem->playerName, results);
}

void l_gethash(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	halo::s_player* player = ReadPlayer(handler, *args[0], true);
	AddResultString(player->hash, results);
}

void l_getteamsize(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	DWORD team = ReadNumber<DWORD>(*args[0]);
	DWORD team_size = 0;
	for (int i = 0; i < 16; i++) {
		halo::s_player* player = halo::game::GetPlayer(i);
		if (player && player->mem->team == team) team_size++;
	}
	AddResultNumber(team_size, results);
}

void l_getplayerobjectid(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	halo::s_player* player = ReadPlayer(handler, *args[0], true);
	if (player->mem->object_id.valid())
		AddResultNumber(player->mem->object_id, results);
	else
		AddResultNil(results);
}

void l_isadmin(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	halo::s_player* player = ReadPlayer(handler, *args[0], true);
	AddResultBool(player->is_admin, results);
}

void l_getadminlvl(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	halo::s_player* player = ReadPlayer(handler, *args[0], true);
	int level;
	if (Admin::getLevel(player->hash, &level))
		AddResultNumber(level, results);
	else AddResultNil(results);
}

void l_setadmin(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	halo::s_player* player = ReadPlayer(handler, *args[0], true);
	player->is_admin = true;
}