#include "Commands.h"
#include "../Common/Streams.h"
#include "../Common/MyString.h"
#include "Logging.h"
#include "Halo/Server/Server.h"
#include "Halo/Game/Game.h"
#include "Halo/Server/Maploader.h"
#include "Halo/AFKDetection.h"
#include "../Scripting.h"
#include <map>
#include <assert.h>
#include "Globals.h"
namespace commands
{
	using namespace halo;

	// ------------------------------------------------------------------------
	// Command handlers

	// sv_logname <type: {Phasor|Script|Game|Rcon}> <new name>
	e_command_result sv_logname(void*, std::vector<std::string>& args, COutStream& out)
	{
		/*if (args.size() != 3) return show_help(SV_LOGNAME, out);

		std::string& log_type = args[1];
		std::string& log_newname = args[2];

		ToLowercase(log_type);

		CLoggingStream* log = NULL;
		if (log_type == "phasor") log = &*g_PhasorLog;
		else if (log_type == "script") log = &*g_ScriptsLog;

		if (!log) return show_help_invalid(SV_LOGNAME, 1, log_type, out);
		log->SetOutFile(WidenString(log_newname));

		return true;*/
		return e_command_result::kProcessed;
	}

	typedef e_command_result (*cmd_func)(void*, CArgParser&, COutStream&);
	static const std::map<std::string, cmd_func> CommandList = []() -> std::map<std::string, cmd_func>
	{
		typedef e_command_result (*cmd_func)(void*, commands::CArgParser&, COutStream&);
		std::map<std::string, cmd_func> cmd;
		cmd["sv_mapcycle_begin"]	= &server::maploader::sv_mapcycle_begin;
		cmd["sv_mapcycle_add"]		= &server::maploader::sv_mapcycle_add;
		cmd["sv_mapcycle_del"]		= &server::maploader::sv_mapcycle_del;
		cmd["sv_mapcycle"]			= &server::maploader::sv_mapcycle;
		cmd["sv_map"]				= &server::maploader::sv_map;
		cmd["sv_end_game"]			= &server::maploader::sv_end_game;
		cmd["sv_kickafk"]			= &afk_detection::sv_kickafk;
		return cmd;
	}();

	// Basic usage description for commands
	static const std::map<std::string, std::string> CommandUsage = []() -> std::map<std::string, std::string>
	{
		std::map<std::string, std::string> usage;
		usage["sv_mapcycle_begin"]	= "";
		usage["sv_mapcycle_add"]	= "<map> <gametype> opt: <script1> <script2> ...";
		usage["sv_mapcycle_del"]	= "<index>";
		usage["sv_mapcycle"]		= "";
		usage["sv_map"]				= usage["sv_mapcycle_add"];
		usage["sv_end_game"]		= "";
		usage["sv_kickafk"]			= "sv_kickafk";
		return usage;
	}();

	//assert(CommandUsage.size() == CommandList.size());

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

		// pass to scripts here
		Scripting::PhasorCaller caller;
		caller.AddArg(exec_player ? exec_player->memory_id : -1);
		caller.AddArg(command);
		Scripting::results_t types = {Common::TYPE_BOOL};
		Scripting::Result result = caller.Call("OnServerCommand", types);
		
		if (result.size() && result.ReadBool().GetValue() == false) 
			return e_command_result::kProcessed;
				
		auto itr = CommandList.find(tokens[0]);
		if (itr != CommandList.end()) {
			try {
				CArgParser args(tokens, tokens[0], 1);
				return itr->second(exec_player, args, out);
			} catch (CArgParserException& e) {
				if (e.has_msg()) out << e.what() << endl;
				auto itr = CommandUsage.find(tokens[0]);
				out << "usage: " << tokens[0] << " " << itr->second << endl;
				return e_command_result::kProcessed;
			}
		}
		return e_command_result::kGiveToHalo;
	}

	// --------------------------------------------------------------------
	const char* CArgParser::k_arg_names[] = {"string", "string", "integer", "number", "player"};

	CArgParser::CArgParser(const std::vector<std::string>& args,
		const std::string& function, size_t start_index) 
		: args(args), function(function), start_index(start_index), index(start_index)
	{
	}

	std::string CArgParser::ReadString(size_t len)
	{
		HasData();
		if (len != -1 && len != args[index].size()) RaiseError(kString, len);
		return args[index++];
	}

	std::wstring CArgParser::ReadWideString(size_t len)
	{
		return WidenString(ReadString(len));
	}

	int CArgParser::ReadInt()
	{
		HasData();
		const char* start = args[index++].c_str();
		char* end;
		int value = strtol(start, &end, 10);
		if (start == end) RaiseError(kInteger); // TEST THIS
		return value;
	}

	unsigned int CArgParser::ReadUInt()
	{
		HasData();
		const char* start = args[index++].c_str();
		char* end;
		unsigned int value = strtoul(start, &end, 10);
		if (start == end) RaiseError(kInteger); // TEST THIS
		return value;
	}

	double CArgParser::ReadDouble()
	{
		HasData();
		const char* start = args[index++].c_str();
		char* end;
		double value = strtod(start, &end);
		if (start == end) RaiseError(kDouble); // TEST THIS
		return value;
	}

	float CArgParser::ReadFloat()
	{
		return (float)ReadDouble();
	}

	halo::s_player& CArgParser::ReadPlayer()
	{
		int playerIndex = ReadInt();
		halo::s_player* player = halo::game::GetPlayerFromRconId(playerIndex);
		if (!player) RaiseError(kPlayer);
		return *player;
	}

	const std::string& CArgParser::ReadPlayerHash()
	{
		halo::s_player& player = ReadPlayer();
		return player.hash;
	}

	// Describes and raises an error
	void CArgParser::RaiseError(e_arg_types expected, int size)
	{
		if (expected == kNone) throw CArgParserException();

		std::string desc;
		switch (expected)
		{
		case kPlayer:
			{
				desc = m_sprintf("%s : argument #%i invalid player.", 
					function.c_str(), index + 1);
			} break;
		default:
			{
				desc = m_sprintf("%s : argument #%i should be of type '%s'",
					function.c_str(), index + 1, k_arg_names[expected]);
				if (size != -1) desc = m_sprintf("%s and length '%i'", desc.c_str(), size);
			} break;
		}
		throw CArgParserException(desc);
	}

}