#include "Commands.h"
#include "../Common/Streams.h"
#include "../Common/MyString.h"
#include "Logging.h"
#include "Halo/Server/Server.h"
#include "Halo/Game/Game.h"
#include "Halo/Server/Maploader.h"
#include "Halo/AFKDetection.h"
#include "../Scripting.h"
#include "LogHandler.h"
#include <map>
#include <assert.h>
#include "Globals.h"

namespace commands
{
	using namespace halo;

	// ------------------------------------------------------------------------
	// Command handlers

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
		cmd["sv_logname"]			= &logging::sv_logname;
		cmd["sv_loglimit"]			= &logging::sv_loglimit;
		cmd["sv_logmovedir"]		= &logging::sv_logmovedir;
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
		usage["sv_kickafk"]			= "<time in minutes>";
		usage["sv_logname"]			= "<log type [phasor,script,game,rcon]> <new name>";
		usage["sv_loglimit"]		= "<log type [phasor,script,game,rcon]> <size in kB>";
		usage["sv_logmovedir"]		= "<directory>";
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
		std::string& command_name = tokens[0];

		// pass to scripts here
		Scripting::PhasorCaller caller;
		caller.AddArg(exec_player ? exec_player->memory_id : -1);
		caller.AddArg(command);
		Scripting::results_t types = {Common::TYPE_BOOL};
		Scripting::Result result = caller.Call("OnServerCommand", types);

		if (result.size() && result.ReadBool().GetValue() == false) 
			return e_command_result::kProcessed;
				
		// Attempt to execute the command, catching any errors resulting
		// from user input.
		auto itr = CommandList.find(command_name);
		if (itr != CommandList.end()) {
			try {				
				CArgParser args(tokens, command_name, 1);
				return itr->second(exec_player, args, out);
			} catch (CArgParserException& e) {
				if (e.has_msg()) out << e.what() << endl;
				auto itr = CommandUsage.find(command_name);
				out << "usage: " << command_name << " " << itr->second << endl;
				return e_command_result::kProcessed;
			}
		}
		return e_command_result::kGiveToHalo;
	}

	// --------------------------------------------------------------------
	const char* CArgParser::k_arg_names[] = {"none", "string", "stringoneof", "integer", "unsigned integer", "number", "player"};

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

	std::string CArgParser::ReadStringOneOf(const std::vector<std::string>& opts,
		bool ignorecase)
	{
		std::string str = ReadString();
		if (ignorecase) ToLowercase(str);
		if (!InVector<std::string>(opts, str)) {
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

	int CArgParser::ReadInt(int min, int max) {	return ReadNumber<int>(kInteger, min,max); }
	unsigned int CArgParser::ReadUInt(unsigned int min, unsigned int max) { return ReadNumber<unsigned int>(kUnsignedInteger,min,max); }
	double CArgParser::ReadDouble(double min, double max) { return ReadNumber<double>(kDouble,min,max); }
	float CArgParser::ReadFloat(float min, float max) {	return (float)ReadDouble(min,max); }

	halo::s_player& CArgParser::ReadPlayer()
	{
		unsigned int playerIndex = ReadUInt()-1;
		halo::s_player* player = halo::game::GetPlayerFromRconId(playerIndex);
		if (!player) {
			index--; // incremented on success in ReadInt
			RaiseError(kPlayer);
		}
		return *player;
	}

	const std::string& CArgParser::ReadPlayerHash()
	{
		halo::s_player& player = ReadPlayer();
		return player.hash;
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
		}
		std::string final_msg = m_sprintf("%s : argument #%u %s", function.c_str(), 
			1 + index - start_index, desc.c_str());
		throw CArgParserException(final_msg);
	}
}