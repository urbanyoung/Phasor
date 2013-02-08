#include "output.h"
#include "../Phasor/Halo/Game/Game.h"
#include "../Phasor/Halo/Server/Server.h"
#include "../Common/MyString.h"
#include "../Phasor/Globals.h"
#include <vector>
#include <string>

using namespace Common;
using namespace Manager;

// Reads the message argument and parses it, replacing {x} with player x's 
// name and splitting the message into a new one at each \n
std::vector<std::wstring> ReadString(Object& obj)
{
	ObjString& str = (ObjString&)obj;
	std::vector<std::wstring> msgs = Tokenize<std::wstring>(WidenString(str.GetValue()), L"\n");

	// Format each message and replace {i} with i's name.
	for (auto itr = msgs.begin(); itr != msgs.end(); ++itr) {
		size_t brace_pos = itr->find(L'{');
		size_t end_brace_pos = itr->find(L'}', brace_pos);

		while (brace_pos != itr->npos && end_brace_pos != itr->npos) {
			size_t diff = end_brace_pos - brace_pos;
			if (diff <= 3) { // ids can only be at most 2 digits				
				std::string str = NarrowString(itr->substr(brace_pos + 1, diff - 1));
				int id;
				/*! \todo
				 * make sure this prints screwed names correctly */
				if (StringToNumber<int>(str, id)) {
					halo::s_player* player = halo::game::GetPlayer(id);
					if (player) {
						itr->erase(brace_pos, diff + 1);
						itr->insert(brace_pos, player->mem->playerName);
					}
				}
			}
			brace_pos = itr->find(L'{', brace_pos + 1);
			end_brace_pos = itr->find(L'}', brace_pos);
		}
	}
	return msgs;	
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
	WriteMessageToStream(g_PrintStream, *args[0]);
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
	COutStream& stream = (player == NULL) ? (COutStream&)g_PrintStream : (COutStream&)*player->console_stream;
	WriteMessageToStream(stream, *args[0]);
}

namespace deprecated
{
	// Don't raise an error if player not found.
	void l_privatesay(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
	{
		halo::s_player* player = ReadPlayer(handler, *args[0], false); 
		if (!player) return;
		privatesay(*player, *args[1]);
	}
}