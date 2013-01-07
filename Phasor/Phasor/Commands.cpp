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
				
		// Attempt to execute the command, catching any errors resulting
		// from user input.
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
	const char* CArgParser::k_arg_names[] = {"none", "string", "stringoneof", "integer", "number", "player"};

	CArgParser::CArgParser(const std::vector<std::string>& args,
		const std::string& function, size_t start_index) 
		: args(args), function(function), start_index(start_index), index(start_index)
	{
	}

	std::string CArgParser::ReadString(size_t len)
	{
		HasData();
		// i also support fixed length strings, so check here
		if (len != -1 && len != args[index].size()) RaiseError(kString, 1, len);
		return args[index++];
	}

	std::wstring CArgParser::ReadWideString(size_t len)
	{
		return WidenString(ReadString(len));
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

	int CArgParser::ReadInt() {	return ReadNumber<int>(kInteger); }
	unsigned int CArgParser::ReadUInt() { return ReadNumber<unsigned int>(kInteger); }
	double CArgParser::ReadDouble()	{ return ReadNumber<double>(kDouble); }
	float CArgParser::ReadFloat() {	return (float)ReadDouble(); }

	halo::s_player& CArgParser::ReadPlayer()
	{
		int playerIndex = ReadInt()-1;
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
	void CArgParser::RaiseError(e_arg_types expected,  double min, double max,
		const std::vector<std::string>* opts)
	{
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
				if (min != 0 || max != 0)
					desc = m_sprintf("expected integer between %i and %i", (int)min, (int)max);
			} break;
		case kDouble:
			{
				if (min != 0 || max != 0)
					desc = m_sprintf("expected number between %.2f and %.2f", min, max);
			} break;
		case kPlayer:
			{
				desc = "invalid player"; 
			} break;
		}
		std::string final_msg = m_sprintf("%s : argument #%i %s", function.c_str(), index,
			desc.c_str());
		throw CArgParserException(final_msg);
	}
}