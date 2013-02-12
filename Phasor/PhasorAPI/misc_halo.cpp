#include "misc_halo.h"
#include "api_readers.h"

using namespace Common;
using namespace Manager;

void l_changeteam(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	halo::s_player* player = ReadPlayer(handler, *args[0], true);
	bool forcekill = ReadBoolean(*args[1]);
	player->ChangeTeam(forcekill);
}

void l_kill(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	halo::s_player* player = ReadPlayer(handler, *args[0], true);
	player->Kill();
}

void l_applycamo(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	halo::s_player* player = ReadPlayer(handler, *args[0], true);
	float duration = ReadNumber<float>(*args[1]);
	player->ApplyCamo(duration);
}

void l_svcmd(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{

}

void l_updateammo(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{

}

void l_setammo(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{

}

void l_setspeed(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	halo::s_player* player = ReadPlayer(handler, *args[0], true);
	float speed = ReadNumber<float>(*args[1]);
	player->SetSpeed(speed);
}