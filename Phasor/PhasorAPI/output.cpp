#include "output.h"
#include "../Phasor/Halo/Game/Game.h"
#include "../Common/MyString.h"
#include "../Phasor/Globals.h"
#include <vector>
#include <string>

using namespace Common;
using namespace Manager;

std::vector<std::string> ReadString(Object& obj)
{
	ObjString& str = (ObjString&)obj;
	return Tokenize<std::string>(str.GetValue(), "\n");
}

// Read the player, if strict is true an error is raised if the player doesn't
// exist.
halo::s_player* ReadPlayer(CallHandler& handler, Object& playerObj, bool strict)
{
	ObjNumber& num = (ObjNumber&)playerObj;
	int player_id = (int)num.GetValue();
	halo::s_player* player = halo::game::GetPlayer(player_id);
	if (!player && strict) {
		std::string err = m_sprintf("valid player required : player %i doesn't exist.",
			player_id);
		handler.RaiseError(err);
	}
	return player;
}

void sendconsoletext(CallHandler& handler, Object& message, Object& playerObj, bool strict=true)
{
	std::vector<std::string> msgs = ReadString(message);
	halo::s_player* player = ReadPlayer(handler, playerObj, strict);
	if (!player) return;
	for (size_t x = 0; x < msgs.size(); x++)
		player->ConsoleMessage(msgs[x]);
}

void l_hprintf(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	// backwards compatibility with old Phasor.
	if (args.size() == 2) return sendconsoletext(handler, *args[0], *args[1], false);
	std::vector<std::string> msgs = ReadString(*args[0]);
	for (size_t x = 0; x < msgs.size(); x++)
		g_PrintStream << msgs[x] << endl;
}

void l_sendconsoletext(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	return sendconsoletext(handler, *args[1], *args[0], true);
}