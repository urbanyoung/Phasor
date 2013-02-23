#pragma once

#include <string>
#include <vector>

class COutStream;
enum e_command_result;

namespace commands { class CArgParser; }

namespace halo { 
	struct s_player;
	namespace server { 
		namespace maploader {
			struct s_phasor_mapcycle_entry;
		}
	namespace mapvote {

	bool GetVoteDecision(maploader::s_phasor_mapcycle_entry& out);
	bool OnServerChat(const halo::s_player& player, const std::wstring& msg);
	void BeginVote();

	// --------------------------------------------------------------------
	//
	e_command_result sv_mapvote(void*, 
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_mapvote_size(void*,
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_mapvote_begin(void*, 
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_mapvote_add(void*, 
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_mapvote_del(void*, 
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_mapvote_list(void*, 
		commands::CArgParser& args, COutStream& out);
}}}

