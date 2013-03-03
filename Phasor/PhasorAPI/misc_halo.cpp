#include "misc_halo.h"
#include "api_readers.h"
#include "../Phasor/Halo/Game/Objects.h"
#include "../Phasor/Globals.h"
#include "../Phasor/Directory.h"
#include "../Common/MyString.h"

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

void l_svcmd(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{ 
	std::string cmd = ReadRawString(*args[0]);
	bool want_result = false;
	if (args.size() == 2) want_result = ReadBoolean(*args[1]);
	if (!want_result) return server::ExecuteServerCommand(cmd);
	
	// we want to store all output so redirect the print stream
	std::list<std::wstring> output;
	std::unique_ptr<CHaloPrintStream> old_print(g_PrintStream.release());

	ProxyRecordStream<CHaloPrintStream>* proxyPrinter = 
		new ProxyRecordStream<CHaloPrintStream>(output);
	g_PrintStream.reset(proxyPrinter);

	// no need to proxy any other streams because the executing player
	// is always reset via ExecuteServerCommand, so only g_PrintStream
	// will be used.

	server::ExecuteServerCommand(cmd);

	std::vector<std::string> narrowed;
	narrowed.reserve(output.size());
	for (auto itr = output.begin(); itr != output.end(); ++itr)
		narrowed.push_back(NarrowString(*itr));

	AddResultTable(narrowed, results);

	g_PrintStream = std::move(old_print);
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