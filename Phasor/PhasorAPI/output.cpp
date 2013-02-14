#include "output.h"
#include "../Phasor/Halo/Game/Game.h"
#include "../Phasor/Halo/Server/Server.h"
#include "../Phasor/Globals.h"
#include "api_readers.h"
#include <vector>
#include <string>

using namespace Common;
using namespace Manager;

void WriteMessageToStream(COutStream& stream, Object& message)
{
	std::vector<std::wstring> msgs = ReadString(message);
	for (size_t x = 0; x < msgs.size(); x++)
		stream << msgs[x] << endl;
}

// Sends console text to the specified player. 
// strict indicates whether or not an error should be raised if the
// player cannot be found.
void sendconsoletext(CallHandler& handler, Object& message, Object& playerObj, bool strict=true)
{
	halo::s_player* player = ReadPlayer(handler, playerObj, strict);
	if (!player) return;
	WriteMessageToStream(*player->console_stream, message);
}

// Print to console
void l_hprintf(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	// backwards compatibility with old Phasor.
	if (args.size() == 2) return sendconsoletext(handler, *args[0], *args[1], false);
	WriteMessageToStream(*g_PrintStream, *args[0]);
}

// Send console text to the specified player (strict)
void l_sendconsoletext(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	return sendconsoletext(handler, *args[1], *args[0], true);
}

// Send message to the specified player
void privatesay(halo::s_player& player, Object& msgObj)
{
	WriteMessageToStream(*player.chat_stream, msgObj);
}

// Send message to specified player, raising an error if player not found.
void l_privatesay(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	//player never null because strict = true
	halo::s_player* player = ReadPlayer(handler, *args[0], true); 
	privatesay(*player, *args[1]);
}

// Send message to entire server
void l_say(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	WriteMessageToStream(halo::server::say_stream, *args[0]);
}

// Respond to person executing the server command or hprintf if no player.
void l_respond(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	halo::s_player* player = halo::server::GetPlayerExecutingCommand();
	COutStream& stream = (player == NULL) ?
		static_cast<COutStream&>(*g_PrintStream) :
		static_cast<COutStream&>(*player->console_stream);
	WriteMessageToStream(stream, *args[0]);
}

void l_log_msg(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	DWORD log_id = ReadNumber<DWORD>(*args[0]);
	COutStream* stream;
	switch (log_id)
	{
	case 1: // game log
		{
			// can't treat the gaming log as a normal stream
			std::vector<std::wstring> msgs = ReadString(*args[1]);
			for (size_t x = 0; x < msgs.size(); x++)
				g_GameLog->WriteLog(kScriptEvent, L"%s", msgs[x].c_str());
			return;
		} break;
	case 2: //phasor log
		{
			stream = g_PhasorLog.get();
		} break;
	case 3: // rcon log
		{
			stream = g_RconLog.get();
		} break;
	case 4: // script log
		{
			stream = g_ScriptsLog.get();
		} break;
	default:
		{
			std::stringstream ss;
			ss << "log_msg : '" << log_id << "' is not a valid id.";
			handler.RaiseError(ss.str());
		} break;
	}
	WriteMessageToStream(*stream, *args[1]);
}