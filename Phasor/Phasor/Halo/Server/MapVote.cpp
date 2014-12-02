#include "MapVote.h"
#include "../../Commands.h"
#include "../../../Common/MyString.h"
#include "../Game/Game.h"
#include "MapLoader.h"
#include "ScriptLoader.h"
#include "Server.h"

namespace halo { namespace server { namespace mapvote {

	struct s_mapvote_entry
	{
		maploader::s_phasor_mapcycle_entry game_info;
		std::string desc;
	};

	struct PlayerVoteHandler
	{
		const halo::s_player* player;
		int vote; // 0-based

		PlayerVoteHandler(const halo::s_player* player)
			: player(player), vote(-1)
		{
		}

		void SetVote(int vote)
		{
			if (this->vote == -1) 
				*player->chat_stream << "Your vote has been received." << endl;
			else
				*player->chat_stream << "Your vote has been updated." << endl;
			this->vote = vote - 1;
		}
	};

	std::vector<s_mapvote_entry> mapvote_list;
	std::vector<PlayerVoteHandler> player_votes;
	std::vector<s_mapvote_entry> current_vote_options;

	static bool mapvote_enabled = false;
	static bool mapvote_in_progress = false;
	static unsigned int mapvote_option_count = 5;

	// --------------------------------------------------------------------
	//
	bool is_valid_vote_index(int vote)
	{
		return vote > 0 && vote <= (int)current_vote_options.size();
	}

	PlayerVoteHandler* GetPlayerVoteHandler(const halo::s_player& player)
	{
		for (auto itr = player_votes.begin(); itr != player_votes.end(); ++itr)
			if (itr->player == &player) return &*itr;
		
		return NULL;
	}

	bool GetVoteDecision(maploader::s_phasor_mapcycle_entry& out)
	{
		if (!mapvote_in_progress) return false;

		std::vector<int> votes(current_vote_options.size(), 0);

		for (auto itr = player_votes.begin(); itr != player_votes.end(); ++itr)
			if (itr->vote != -1) votes[itr->vote]++;
		
		// Make a list of all options that were voted for the most
		std::vector<int> max_voted;
		for (size_t x = 0; x < votes.size(); x++)
		{
			if (!max_voted.size()) max_voted.push_back(x);
			else if (votes[max_voted[0]] == votes[x])
				max_voted.push_back(x);
			else if (votes[max_voted[0]] < votes[x]) {
				max_voted.clear();
				max_voted.push_back(x);
			}
		}

		int vote_index = max_voted[rand() % max_voted.size()];
		out = current_vote_options[vote_index].game_info;
		mapvote_in_progress = false;

		return true;
	}

	bool OnServerChat(const halo::s_player& player, const std::wstring& msg)
	{
		if (!mapvote_in_progress) return true; 

		std::vector<std::wstring> tokens = Tokenize<std::wstring>(msg, L" ");
		size_t size = tokens.size();
		if (size == 0) return true;

		bool vote_processed = false;

		int vote = -1;
		if (size == 1)
			StringToNumber<int>(NarrowString(tokens[0]), vote);
		else if (size == 2) {
			if (tokens[0] == L"vote" || tokens[0] == L"vtoe" || tokens[0] == L"voet")
				StringToNumber<int>(NarrowString(tokens[1]), vote);
		}

		if (is_valid_vote_index(vote))	{
			PlayerVoteHandler* handler = GetPlayerVoteHandler(player);
			if (handler) {
				handler->SetVote(vote);
				vote_processed = true;
			}			
		}
		return !vote_processed;
	}

	void BeginVote()
	{
		if (!mapvote_enabled) return;

		player_votes.clear();
		current_vote_options.clear();

		for (int i = 0; i < 16; i++) {
			s_player* player = game::getPlayer(i);
			if (player) player_votes.push_back(PlayerVoteHandler(player));
		}

		// Check if we need to randomly select the maps
		if (mapvote_list.size() > mapvote_option_count)
		{
			// inefficient but idgaf
			auto tmpList = mapvote_list;

			for (size_t i = 0; i < mapvote_option_count; i++)	{
				int rindex = rand() %  tmpList.size();
				current_vote_options.push_back(tmpList[rindex]);
				tmpList.erase(tmpList.begin() + rindex);
			}
		}
		else
			current_vote_options = mapvote_list;
		
		for (size_t x = 0; x < current_vote_options.size(); x++) {
			s_mapvote_entry& entry = current_vote_options[x];
			say_stream.print("[%i] %s", x + 1, entry.desc.c_str());
		}

		mapvote_in_progress = true;
	}

	e_command_result sv_mapvote(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		bool enable = args.ReadBool();
		if (enable && !mapvote_list.size()) {
			out << "You need to add some options before enabling, see sv_mapvote_add." << endl;
			return e_command_result::kProcessed;
		}
		mapvote_enabled = enable;
		out << "Map voting has been " << (mapvote_enabled ? "enabled" : "disabled")
			<< endl;
		return e_command_result::kProcessed;
	}

	e_command_result sv_mapvote_size(void*,
		commands::CArgParser& args, COutStream& out)
	{
		if (mapvote_in_progress)
			out << "You must wait until the current vote has finished." << endl;
		else {
			mapvote_option_count = args.ReadUInt(2);
			out << "Only " << (int)mapvote_option_count << " options will be displayed." << endl;
		}
		return e_command_result::kProcessed;
	}

	e_command_result sv_mapvote_begin(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		if (!mapvote_enabled) {
			out << "Map voting isn't enabled, use sv_mapvote true to enable." << endl;
		} else if (!mapvote_list.size()) {
			out << "There aren't any votable maps, use sv_mapvote_add to add maps." << endl;
		} else {
			if (maploader::LoadGame(mapvote_list[0].game_info, out))
				out << "The map voting process is beginning.." << endl;
			else
				out << "An error occurred, can't start map vote." << endl;
		}
		return e_command_result::kProcessed;
	}

	e_command_result sv_mapvote_add(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		s_mapvote_entry entry;
		entry.game_info.map = args.ReadString();
		entry.game_info.gametype = args.ReadWideString();
		entry.desc = args.ReadString();

		for (size_t x = 3; x < args.size(); x++)
			entry.game_info.scripts.push_back(args.ReadString());
		
		if (!maploader::ValidateUserInput(entry.game_info, out))
			return e_command_result::kProcessed;
		
		mapvote_list.push_back(entry);
		return e_command_result::kProcessed;
	}

	e_command_result sv_mapvote_del(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		size_t index = args.ReadUInt(1, mapvote_list.size());
		mapvote_list.erase(mapvote_list.begin() + index-1);
		if (!mapvote_list.size()) {
			out << "No more votable maps. Map voting disabled." << endl;
			mapvote_enabled = false;
		}
		return e_command_result::kProcessed;
	}

	e_command_result sv_mapvote_list(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		out.wprint(L"   %-20s%-20s%s", L"Map", L"Variant", L"Description");
		const wchar_t* fmt = L"%-3i%-20s%-20s%s";

		for (size_t x = 0; x < mapvote_list.size(); x++)
		{
			s_mapvote_entry& entry = mapvote_list[x];
			out.wprint(fmt, x + 1, WidenString(entry.game_info.map).c_str(),
				entry.game_info.gametype.c_str(),
				WidenString(entry.desc).c_str());
		}
		return e_command_result::kProcessed;
	}

}}}