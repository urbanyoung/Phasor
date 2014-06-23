#include "misc_halo.h"
#include "api_readers.h"
#include "../../Phasor/Halo/Game/Objects.h"
#include "../../Phasor/Globals.h"
#include "../../Phasor/Directory.h"
#include "../../Common/MyString.h"

using namespace Common;
using namespace Manager;
using namespace halo;
using namespace objects;

void l_changeteam(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	s_player* player = ReadPlayer(handler, *args[0], true);
	bool forcekill = ReadBoolean(*args[1]);
	BYTE team = !player->mem->team;
	if (args.size() == 3) team = ReadNumber<BYTE>(*args[2]);
	player->ChangeTeam(team, forcekill);
}

void l_kill(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	s_player* player = ReadPlayer(handler, *args[0], true);
	player->Kill();
}

void l_applycamo(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	s_player* player = ReadPlayer(handler, *args[0], true);
	float duration = ReadNumber<float>(*args[1]);
	player->ApplyCamo(duration);
}

void svcmd_redirect(Object::unique_list& results, const std::string& cmd,
	bool want_result, halo::s_player* player, COutStream& baseStream)
{
	if (!want_result) return server::ExecuteServerCommand(cmd, player);

	RecordStream record;
	NotifyStream _(baseStream, record);

	server::ExecuteServerCommand(cmd, player);

	const std::list<std::wstring>& output = record.getRecord();
	std::vector<std::string> narrowed;
	narrowed.reserve(output.size());
	for (auto itr = output.cbegin(); itr != output.cend(); ++itr)
		narrowed.push_back(NarrowString(*itr));

	AddResultTable(narrowed, results);
}

// will change this when i rework the script system
void check_script_reload(CallHandler& handler, const std::string& cmd)
{
	std::vector<std::string> tokens = TokenizeArgs(cmd);
	if (tokens.size() && tokens[0] == "sv_script_reload")
		handler.RaiseError("scripts cannot execute sv_script_reload.");

}
void l_svcmd(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{ 
	std::string cmd = ReadRawString(*args[0]);
	bool want_result = false;
	if (args.size() == 2) want_result = ReadBoolean(*args[1]);

	check_script_reload(handler, cmd);
	return svcmd_redirect(results, cmd, want_result, NULL, *g_PrintStream);
}

void l_svcmdplayer(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	bool want_result = false;
	std::string cmd = ReadRawString(*args[0]);
	halo::s_player* player = ReadPlayer(handler, *args[1], true);
	if (args.size() == 3) want_result = ReadBoolean(*args[2]);
	check_script_reload(handler, cmd);
	return svcmd_redirect(results, cmd, want_result, player, *player->console_stream);
}

void l_updateammo(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	ident weapon_id = make_ident(ReadNumber<DWORD>(*args[0]));
	s_halo_weapon* weapon = (s_halo_weapon*)GetObjectAddress(weapon_id);
	if (!weapon) handler.RaiseError("updateammo: invalid weapon id!");
	weapon->SyncAmmo(weapon_id);
}

void l_setammo(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	ident weapon_id = make_ident(ReadNumber<DWORD>(*args[0]));
	WORD clip_ammo = ReadNumber<WORD>(*args[1]);
	WORD pack_ammo = ReadNumber<WORD>(*args[2]);
	s_halo_weapon* weapon = (s_halo_weapon*)GetObjectAddress(weapon_id);
	
	if (!weapon) handler.RaiseError("setammo: invalid weapon id!");
	weapon->SetAmmo(pack_ammo, clip_ammo);
	weapon->SyncAmmo(weapon_id);
}

void l_setspeed(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	s_player* player = ReadPlayer(handler, *args[0], true);
	float speed = ReadNumber<float>(*args[1]);
	player->SetSpeed(speed);
}

void l_getprofilepath(CallHandler& handler, Object::unique_deque&, Object::unique_list& results)
{
	AddResultString(g_ProfileDirectory, results);
}

void l_getservername(CallHandler& handler, Object::unique_deque&, Object::unique_list& results)
{
	halo::server::s_server_info* server = server::GetServerStruct();
	AddResultString(server->server_name, results);
}