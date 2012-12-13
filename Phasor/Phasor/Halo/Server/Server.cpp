#include "Server.h"
#include "../Game/Maps.h"
#include "../../../Common/MyString.h"
#include "Common.h"
#include <vector>

namespace halo { namespace server
{
	struct PhasorCommands
	{
		(bool)(*fn)(void*, std::vector<std::string>&);
		const char* key;
	};

	// Called when a map is being loaded
	bool __stdcall OnMapLoad(BYTE* mapData)
	{
		return game::maps::OnMapLoad(mapData);
	}

	// Called when a console command is to be executed
	// true: Event has been handled, don't pass to server
	// false: Not handled, pass to server.
	bool __stdcall ProcessCommand(char* command)
	{
		std::vector<std::string> args = TokenizeArgs(command);
		if (args.size() == 0) return true; // nothing to process
		/*g_PrintStream << L"Executing command '" << args[0] << "' from '" <<
			command << "'" << endl;*/
		return false;
	}
}}