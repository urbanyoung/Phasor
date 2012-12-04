#include "Commands.h"
#include "../Common/Streams.h"
#include "../Common/MyString.h"
#include "Logging.h"

// Invalid command usage stuff.
struct s_cmdhelp
{
	wchar_t* funcname;
	wchar_t* usage;
};

enum cmdhelpindex
{
	SV_LOGNAME = 0
};

static const s_cmdhelp cmdhelptable[] =
{
	{L"sv_logname", L"sv_logname <type: {phasor|script|game|rcon}> <new name>"}
};

bool show_help(cmdhelpindex index, COutStream& out)
{
	out << L"usage: " << cmdhelptable[index].usage << endl;
	return false;
}

bool show_help_invalid(cmdhelpindex index, int arg_number, const std::string& arg,
	COutStream& out)
{
	out << cmdhelptable[index].funcname << 
		L" : invalid argument #" << arg_number << L" '" << arg << L"'" << endl;
	show_help(index, out);
	return false;
}

// ------------------------------------------------------------------------
// Command handlers

// sv_logname <type: {Phasor|Script|Game|Rcon}> <new name>
bool sv_logname(std::vector<std::string>& args, COutStream& out)
{
	if (args.size() != 3) return show_help(SV_LOGNAME, out);

	std::string& log_type = args[1];
	std::string& log_newname = args[2];

	ToLowercase(log_type);

	CLoggingStream* log = NULL;
	if (log_type == "phasor") log = &*g_PhasorLog;
	else if (log_type == "script") log = &*g_ScriptsLog;

	if (!log) return show_help_invalid(SV_LOGNAME, 1, log_type, out);
	log->SetOutFile(WidenString(log_newname));

	return true;
}

struct CommandEntry
{
	const char* name;
	bool (*func)(std::vector<std::string>&, COutStream&);
};

static const CommandEntry CommandList[] =
{
	{"sv_logname", &sv_logname},
	{NULL, NULL}
};

// Returns success or failure
bool ProcessCommand(const std::string& command, COutStream& out)
{
	std::vector<std::string> tokens = TokenizeArgs(command);
	if (tokens.size() == 0) return false;

	for (size_t x = 0; CommandList[x].name; x++) {
		if (CommandList[x].name == tokens[0]) {
			return CommandList[x].func(tokens, out);
		}
	}

	return false;
}