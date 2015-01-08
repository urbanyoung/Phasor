#include "Commands.h"
#include "../Common/Streams.h"
#include "../Common/MyString.h"

#include "Logging.h"
#include "Halo/Server/Server.h"
#include "Halo/Game/Game.h"
#include "Halo/Server/Maploader.h"
#include "Halo/Server/ScriptLoader.h"
#include "Halo/Server/MapVote.h"
#include "Halo/Server/misc_cmds.h"
#include "Halo/AFKDetection.h"
#include "Halo/Alias.h"
#include "../Scripts/script-events.h"
#include "LogHandler.h"
#include "Admin.h"
#include <map>
#include <assert.h>
#include "Globals.h"

namespace commands
{
	using namespace halo;

	// ------------------------------------------------------------------------
	// Command handlers

	typedef e_command_result (*cmd_func)(void*, CArgParser&, COutStream&);
	static const std::map<std::string, cmd_func> commandList = []() -> std::map<std::string, cmd_func>
	{
		typedef e_command_result (*cmd_func)(void*, commands::CArgParser&, COutStream&);
		std::map<std::string, cmd_func> cmd;
		cmd["quit"]					= &server::sv_quit;
		cmd["sv_mapcycle_begin"]	= &server::maploader::sv_mapcycle_begin;
		cmd["sv_mapcycle_add"]		= &server::maploader::sv_mapcycle_add;
		cmd["sv_mapcycle_del"]		= &server::maploader::sv_mapcycle_del;
		cmd["sv_mapcycle"]			= &server::maploader::sv_mapcycle;
		cmd["sv_map"]				= &server::maploader::sv_map;
		cmd["sv_end_game"]			= &server::maploader::sv_end_game;
       // cmd["sv_refresh_maps"] = &server::maploader::sv_refresh_maps;

		cmd["sv_mapvote"]			= &server::mapvote::sv_mapvote;
		cmd["sv_mapvote_size"]		= &server::mapvote::sv_mapvote_size;
		cmd["sv_mapvote_begin"]		= &server::mapvote::sv_mapvote_begin;
		cmd["sv_mapvote_add"]		= &server::mapvote::sv_mapvote_add;
		cmd["sv_mapvote_del"]		= &server::mapvote::sv_mapvote_del;
		cmd["sv_mapvote_list"]		= &server::mapvote::sv_mapvote_list;

		/*! \todo change Open/Close/Reload script to take an output stream. */
		cmd["sv_script_reload"]		= &server::scriptloader::sv_script_reload;
		cmd["sv_script_load"]		= &server::scriptloader::sv_script_load;
		cmd["sv_script_unload"]		= &server::scriptloader::sv_script_unload;
		cmd["sv_script_list"]		= &server::scriptloader::sv_script_list;
		
		cmd["sv_kickafk"]			= &afk_detection::sv_kickafk;		

		// misc
		cmd["sv_version"]			= &server::misc::sv_version;
		cmd["sv_version_check"]		= &server::misc::sv_version_check;
		cmd["sv_hash_check"]		= &server::misc::sv_hash_check;
		cmd["sv_kill"]				= &server::misc::sv_kill;
		cmd["sv_getobject"]			= &server::misc::sv_getobject;
		cmd["sv_invis"]				= &server::misc::sv_invis;
		cmd["sv_setspeed"]			= &server::misc::sv_setspeed;
		cmd["sv_say"]				= &server::misc::sv_say;
		cmd["sv_gethash"]			= &server::misc::sv_gethash;
		cmd["sv_changeteam"]		= &server::misc::sv_changeteam;

		cmd["sv_log_name"]			= &logging::sv_logname;
		cmd["sv_log_limit"]			= &logging::sv_loglimit;
		cmd["sv_log_move_dir"]		= &logging::sv_logmovedir;

		cmd["sv_alias_enable"]		= &alias::sv_alias_enable;
		cmd["sv_alias_hash"]		= &alias::sv_alias_hash;
		cmd["sv_alias_search"]		= &alias::sv_alias_search;

		cmd["sv_admin_add"]			= &Admin::sv_admin_add;
		cmd["sv_admin_del"]			= &Admin::sv_admin_del;
		cmd["sv_admin_list"]		= &Admin::sv_admin_list;
		cmd["sv_admin_cur"]			= &Admin::sv_admin_cur;
		cmd["sv_admin_reload"]		= &Admin::sv_admin_reload;
		cmd["sv_admin_commands"]	= &Admin::sv_commands;
		cmd["sv_admin_check"]		= &Admin::sv_admin_check;
		cmd["sv_public"]			= &Admin::sv_public;
		return cmd;
	}();

	// Basic usage description for commands
	static const std::map<std::string, std::string> commandUsage = []() -> std::map<std::string, std::string>
	{
		std::map<std::string, std::string> usage;
		usage["sv_mapcycle_begin"]	= "";
		usage["sv_mapcycle_add"]	= "<map> <gametype> opt: [script1] [script2] ...";
		usage["sv_mapcycle_del"]	= "<index>";
		usage["sv_mapcycle"]		= "";
		usage["sv_map"]				= usage["sv_mapcycle_add"];
		usage["sv_end_game"]		= "";

		usage["sv_mapvote"]			= "<true or false>";
		usage["sv_mapvote_size"]	= "<number of options to show>";
		usage["sv_mapvote_begin"]	= "";
		usage["sv_mapvote_add"]		= "<map> <gametype> <description> opt: [script1] [script2] ...";
		usage["sv_mapvote_del"]		= "<index from sv_mapvote_list>";
		usage["sv_mapvote_list"]	= "";

		usage["sv_script_reload"]	= "[script to reload]";
		usage["sv_script_load"]		= "<script to load> <persistent>";
		usage["sv_script_unload"]	= "<script>";
		usage["sv_script_list"]		= "";

		usage["sv_kickafk"]			= "<time in minutes>";
		usage["sv_version"]			= "<version> see `sv_version` for list of versions.";
		usage["sv_version_check"]	= "<true or false>";
		usage["sv_hash_check"]		= "<true or false>";
		usage["sv_kill"]			= "<player>";
		usage["sv_getobject"]		= "<object id>";
		usage["sv_invis"]			= "<player> <duration>";
		usage["sv_setspeed"]		= "<player> <speed>";
		usage["sv_say"]				= "<msg to say>";
		usage["sv_gethash"]			= "<player>";
		usage["sv_changeteam"]		= "<player>";

		usage["sv_log_name"]		= "<log type [phasor,script,game,rcon]> <new name>";
		usage["sv_log_limit"]		= "<log type [phasor,script,game,rcon]> <size in kB>";
		usage["sv_logmove_dir"]		= "<directory>";

		usage["sv_alias_enable"]	= "<status>";
		usage["sv_alias_hash"]		= "<hash or player index>";
		usage["sv_alias_search"]	= "<partial name to find>";

		usage["sv_admin_add"]		= "<player number or hash> <auth name> <level>";
		usage["sv_admin_del"]		= "<admin's name>";
		usage["sv_admin_list"]		= "";
		usage["sv_admin_cur"]		= "";
		usage["sv_admin_reload"]	= "";
		usage["sv_admin_commands"]	= "";
		usage["sv_admin_check"]		= "<true or false>";
		usage["sv_public"]			= "<true or false>";
		return usage;
	}();

	class CArgParserException
	{
	private:
		std::string err;
	public:
		CArgParserException() {}
		CArgParserException(const std::string& err) : err(err) {}
		const char* what() { return err.c_str(); }
		inline bool has_msg() { return err.size() != 0; }
	};

	// Returns success or failure
	e_command_result ProcessCommand(const std::string& command, 
		COutStream& out, halo::s_player* exec_player)
	{
		std::vector<std::string> tokens = TokenizeArgs(command);
		if (tokens.size() == 0) return e_command_result::kProcessed;
		std::string& command_name = tokens[0];

		bool do_process = scripting::events::OnServerCommand(exec_player, command);
		if (!do_process) return e_command_result::kProcessed;
				
		// Attempt to execute the command, catching any errors resulting
		// from user input.
		auto itr = commandList.find(command_name);
		if (itr != commandList.end()) {
			try {				
				CArgParser args(tokens, command_name, 1);
				return itr->second(exec_player, args, out);
			} catch (CArgParserException& e) {
				if (e.has_msg()) out << e.what() << endl;
				auto itr = commandUsage.find(command_name);
				if (itr != commandUsage.end())
					out << "usage: " << command_name << " " << itr->second << endl;
				else 
					out << "bug: implement usage help for " << command_name << endl;
				return e_command_result::kProcessed;
			}
		}
		return e_command_result::kGiveToHalo;
	}

	// --------------------------------------------------------------------
	const char* CArgParser::k_arg_names[] = {"none", "string", "stringoneof", 
		"integer", "positive integer", "number", "boolean", "player", "player or hash"};


	CArgParser::CArgParser(const std::vector<std::string>& args,
		const std::string& function, size_t start_index) 
		: args(args), function(function), start_index(start_index), index(start_index)
	{
	}

	std::string CArgParser::ReadString(size_t min, size_t max)
	{
		HasData();
		size_t len = args[index].size();
		if (len < min || (len > max && max != 0)) RaiseError(kString, min, max);
		return args[index++];
	}

	std::wstring CArgParser::ReadWideString(size_t min, size_t max)
	{
		return WidenString(ReadString(min,max));
	}	

	bool CArgParser::InVector(const std::vector<std::string>& opts, 
		const std::string& to_check)
	{
		for (size_t x = 0; x < opts.size(); x++)
			if (opts[x] == to_check) return true;
		return false;
	}

	std::string CArgParser::ReadStringOneOf(const std::vector<std::string>& opts,
		bool ignorecase)
	{
		std::string str = ReadString();
		if (ignorecase) ToLowercase(str);
		if (!InVector(opts, str)) {
			index--; // incremented on success in ReadString
			RaiseError(kStringOneOf, 0, 0, &opts);
		}
		return str;
	}

	std::wstring CArgParser::ReadWideStringOneOf(const std::vector<std::string>& opts,
		bool ignorecase)
	{
		return WidenString(ReadStringOneOf(opts, ignorecase));
	}

	template <class T>
	T CArgParser::ReadNumber(e_arg_types type, T min, T max)
	{
		HasData();
		T value;
		if (!StringToNumber<T>(args[index], value) || value < min || value > max)
			RaiseError(type, min, max);
		index++;
		return value;
	}

	int CArgParser::ReadInt(int min, int max) {	return ReadNumber<int>(kInteger, min,max); }
	unsigned int CArgParser::ReadUInt(unsigned int min, unsigned int max) { return ReadNumber<unsigned int>(kUnsignedInteger,min,max); }
	double CArgParser::ReadDouble(double min, double max) { return ReadNumber<double>(kDouble,min,max); }
	float CArgParser::ReadFloat(float min, float max) {	return (float)ReadDouble(min,max); }

	bool CArgParser::ReadBool()
	{
		HasData();
		bool b = false;
		if (args[index] == "true" || args[index] == "1")
			b = true;
		else if (args[index] == "false" || args[index] == "0")
			b = false;
		else
			RaiseError(kBoolean);
		return b;
	}
	halo::s_player& CArgParser::ReadPlayer()
	{
		unsigned int playerIndex = ReadUInt()-1;
		halo::s_player* player = halo::game::getPlayerFromRconId(playerIndex);
		if (!player) {
			index--; // incremented on success in ReadInt
			RaiseError(kPlayer);
		}
		return *player;
	}

	std::string CArgParser::ReadPlayerHash()
	{
		halo::s_player& player = ReadPlayer();
		return player.hash;
	}

	std::string CArgParser::ReadPlayerOrHash()
	{
		HasData();
		const std::string& str = args[index++];
		if (str.size() == 32) return str; // they entered the hash
		unsigned int playerIndex;
		if (!::StringToNumber<unsigned int>(str, playerIndex)) RaiseError(kPlayerOrHash);
		halo::s_player* player = halo::game::getPlayerFromRconId(playerIndex-1);
		if (!player) RaiseError(kPlayerOrHash);
		return player->hash;
	}

	// Describes and raises an error
	void CArgParser::RaiseError(e_arg_types expected,  double min, double max,
		const std::vector<std::string>* opts)
	{
		// no description available
		if (expected == kNone) throw CArgParserException();

		// default message
		std::string desc = m_sprintf("should be of type '%s'", k_arg_names[expected]);
		switch (expected)
		{
		case kString:
			{
				if (min != 0 && max == 0)
					desc = m_sprintf("expected string with minimum length %i", min);
				else if (min != 0 && max != 0)
					desc = m_sprintf("expected string with length between %i and %i characters.", min, max);
				else if (min == 0 && max != 0)
					desc = m_sprintf("expected string with maximum length %i", max);
			} break;
		case kStringOneOf:
			{
				desc.reserve(100);
				desc = "should be one of: ";
				for (size_t x = 0; x < opts->size(); x++) {
					if (x != 0) desc += ",";
					desc += "'" + (*opts)[x] + "'";
				}
			} break;
		case kInteger:
			{
				if (min != INT_MIN && max == INT_MAX)
					desc = m_sprintf("expected integer >= %i", (int)min);
				else if (min != INT_MIN && max != INT_MAX)
					desc = m_sprintf("expected integer between %i and %i", (int)min, (int)max);
				else if (min == INT_MIN && max != INT_MAX)
					desc = m_sprintf("expected integer <= %i", (int)max);
			} break;
		case kUnsignedInteger:
			{
				if (min != 0 && max == UINT_MAX)
					desc = m_sprintf("expected positive integer >= %u", (unsigned int)min);
				else if (min != 0 && max != UINT_MAX)
					desc = m_sprintf("expected positive integer between %u and %u", (unsigned int)min, (unsigned int)max);
				else if (min == 0 && max != UINT_MAX)
					desc = m_sprintf("expected positive integer <= %u", (unsigned int)max);
			} break;
		case kDouble:
			{
				if (min != DBL_MIN && max == DBL_MAX)
					desc = m_sprintf("expected number >= %.2f", min);
				else if (min != DBL_MIN && max != DBL_MAX)
					desc = m_sprintf("expected number between %.2f and %.2f", min, max);
				else if (min == DBL_MIN && max != DBL_MAX)
					desc = m_sprintf("expected number <= %.2f", max);
			} break;
		case kPlayer:
			{
				desc = "invalid player"; 
			} break;
		case kPlayerOrHash:
			{
				desc = "expected player or hash";
			} break;
		}
		std::string final_msg = m_sprintf("%s : argument #%u %s", function.c_str(), 
			1 + index - start_index, desc.c_str());
		throw CArgParserException(final_msg);
	}
}