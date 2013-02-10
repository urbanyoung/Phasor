#include "api_readers.h"
#include "../Common/MyString.h"

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
			if (diff == 2 || diff == 3) { // ids can only be at most 2 digits				
				std::string str = NarrowString(itr->substr(brace_pos + 1, diff - 1));
				int id;
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

// Don't apply any processing like in ReadString
std::string ReadRawString(Common::Object& obj)
{
	ObjString& str = (ObjString&)obj;
	return str.GetValue();
}

// Raises an invalid player error
void RaisePlayerError(Manager::CallHandler& handler, int player_id)
{
	std::string err = m_sprintf("valid player required : player %i doesn't exist.",
		player_id);
	handler.RaiseError(err);
}

// Read the player, if strict is true an error is raised if the player doesn't
// exist.
halo::s_player* ReadPlayer(CallHandler& handler, Object& playerObj, bool strict)
{
	int player_id = ReadNumber<int>(playerObj);
	halo::s_player* player = halo::game::GetPlayer(player_id);
	if (!player && strict) RaisePlayerError(handler, player_id);
	return player;
}

void AddResultNil(Common::Object::unique_list& results)
{
	results.push_back(std::unique_ptr<Object>(new Object()));
}

void AddResultString(const std::string& str, Object::unique_list& results)
{
	results.push_back(std::unique_ptr<Object>(new ObjString(str)));
}

void AddResultString(const std::wstring& str, Object::unique_list& results)
{
	return AddResultString(NarrowString(str), results);
}

void AddResultNumber(double value, Object::unique_list& results)
{
	results.push_back(std::unique_ptr<Object>(new ObjNumber(value)));
}

void AddResultBool(bool b, Object::unique_list& results)
{
	results.push_back(std::unique_ptr<Object>(new ObjBool(b)));
}

void AddResultPtr(void* ptr, Object::unique_list& results)
{
	results.push_back(std::unique_ptr<Object>(new ObjNumber((DWORD)ptr)));
}

void AddResultTable(const std::vector<std::string>& data, Object::unique_list& results)
{
	results.push_back(std::unique_ptr<Object>(new ObjTable(data)));
}