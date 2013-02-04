#include "LogHandler.h"
#include "../Common/MyString.h"
#include "Logging.h"
#include "Globals.h"
#include "../Common/FileIO.h"

namespace logging
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

	// returns null on invalid log type.. should be checked before calling
	CLoggingStream* GetLogFromTypeName(const std::string& log_type)
	{
		CLoggingStream* log = NULL;
		if (log_type == "phasor") log = &*g_PhasorLog;
		else if (log_type == "script") log = &*g_ScriptsLog;
		else if (log_type == "game") log = &g_GameLog->GetLogStream();
		else if (log_type == "rcon") log = &*g_RconLog;
		return log;
	}

	e_command_result sv_logname(void*, CArgParser& args, COutStream& out)
	{
		std::string log_type = args.ReadStringOneOf(log_types, true);
		std::wstring log_newname = args.ReadWideString();
		
		//log_type validated above
		CLoggingStream* log = GetLogFromTypeName(log_type);
		log->SetOutFile(log_newname);
		out << "The " << log_type << " log will now be saved as '" << log_newname << "'" << endl;
		return e_command_result::kProcessed;
	}

	e_command_result sv_loglimit(void*, CArgParser& args, COutStream& out)
	{
		std::string log_type = args.ReadStringOneOf(log_types, true);
		unsigned int max_size = args.ReadUInt(1);
		//log_type validated above
		CLoggingStream* log = GetLogFromTypeName(log_type);
		log->SetMoveSize(max_size);
		out << "The " << log_type << " log will be moved when it reaches " << 
			(DWORD)max_size << " kB" << endl;		
		return e_command_result::kProcessed;
	}

	e_command_result sv_logmovedir(void*, CArgParser& args, COutStream& out)
	{
		std::wstring new_move_dir = args.ReadWideString();
		if (!NDirectory::IsDirectory(new_move_dir)) {
			out << "'" << new_move_dir << "' isn't a valid directory." << endl;
			return e_command_result::kProcessed;
		}
		g_PhasorLog->SetMoveDirectory(new_move_dir);
		g_ScriptsLog->SetMoveDirectory(new_move_dir);
		g_GameLog->GetLogStream().SetMoveDirectory(new_move_dir);
		g_RconLog->SetMoveDirectory(new_move_dir);
		out << "Logs will be moved to '" << new_move_dir << "' if they exceed the size limit" << endl;
		return e_command_result::kProcessed;
	}
}