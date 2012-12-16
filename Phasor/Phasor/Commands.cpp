#include "Commands.h"
#include "../Common/Streams.h"
#include "../Common/MyString.h"
#include "Logging.h"
#include "Halo/Server/Server.h"
#include "Halo/Server/Maploader.h"

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

	struct CommandEntry
	{
		e_command_result (*func)(void*, std::vector<std::string>&, COutStream&);
		const char* name;	
	};

	static const CommandEntry CommandList[] =
	{
		{&sv_logname, "sv_logname"},
		{&server::maploader::sv_mapcycle_begin, "sv_mapcycle_begin"},
		{&server::maploader::sv_mapcycle_add, "sv_mapcycle_add"},
		{&server::maploader::sv_mapcycle_del, "sv_mapcycle_del"},
		{&server::maploader::sv_mapcycle, "sv_mapcycle"},
		{&server::maploader::sv_map, "sv_map"},
		{&server::maploader::sv_end_game, "sv_end_game"},
		{NULL, NULL}
	};

	// Returns success or failure
	e_command_result ProcessCommand(const std::string& command, COutStream& out,
		void* exec_player)
	{
		std::vector<std::string> tokens = TokenizeArgs(command);
		if (tokens.size() == 0) return e_command_result::kProcessed;

		for (size_t x = 0; CommandList[x].name; x++) {
			if (CommandList[x].name == tokens[0]) {
				return CommandList[x].func(exec_player, tokens, out);
			}
		}

		return e_command_result::kGiveToHalo;
	}
}