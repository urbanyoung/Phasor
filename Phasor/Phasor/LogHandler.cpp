#include "LogHandler.h"
#include "../Common/MyString.h"
#include "Logging.h"
#include "Globals.h"

namespace logging
{
	e_command_result sv_logname(void*, CArgParser& args, COutStream& out)
	{
		static const std::vector<std::string> log_types = []() -> std::vector<std::string>
		{
			std::vector<std::string> types;
			types.push_back("phasor");
			types.push_back("script");
			types.push_back("game");
			types.push_back("rcon");
			return types;
		}();

		std::string log_type = args.ReadStringOneOf(log_types, true);
		std::wstring log_newname = args.ReadWideString();
		ToLowercase(log_type);

		CLoggingStream* log = NULL;
		if (log_type == "phasor") log = &*g_PhasorLog;
		else if (log_type == "script") log = &*g_ScriptsLog;
		else if (log_type == "game") log = &g_GameLog->GetLogStream();
		else if (log_type == "rcon") log = &*g_RconLog;
		
		//log will never be null
		log->SetOutFile(log_newname);

		out << "The log will now be saved as '" << log_newname << "'" << endl;
		return e_command_result::kProcessed;
	}
}