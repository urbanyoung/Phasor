#include "output.h"
#include "../Phasor/Halo/Game/Game.h"
#include "../Common/MyString.h"
#include "../Phasor/Globals.h"
#include <vector>
#include <string>

using namespace Common;
using namespace Manager;

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

void sendconsoletext(CallHandler& handler, Object& message, Object& playerObj, bool strict=true)
{
	std::vector<std::wstring> msgs = ReadString(message);
	halo::s_player* player = ReadPlayer(handler, playerObj, strict);
	if (!player) return;
	for (size_t x = 0; x < msgs.size(); x++)
		player->ConsoleMessage(msgs[x]);
}

void l_hprintf(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	// backwards compatibility with old Phasor.
	if (args.size() == 2) return sendconsoletext(handler, *args[0], *args[1], false);
	std::vector<std::wstring> msgs = ReadString(*args[0]);
	for (size_t x = 0; x < msgs.size(); x++)
		g_PrintStream << msgs[x] << endl;
}

void l_sendconsoletext(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	return sendconsoletext(handler, *args[1], *args[0], true);
}