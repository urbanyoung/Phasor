#include "Server.h"
#include "../../../Common/MyString.h"
#include "Common.h"
#include "../../Logging.h"
#include <vector>

namespace halo { namespace server
{
	std::string current_map_base;

	struct PhasorCommands
	{
		(bool)(*fn)(void*, std::vector<std::string>&);
		const char* key;
	};

	// Called when a map is being loaded
	bool __stdcall OnMapLoad(maploader::s_mapcycle_entry* loading_map)
	{
		bool bMapUnchanged = true;
		char* map = loading_map->map;
		char* gametype = loading_map->gametype;

#ifdef PHASOR_PC		
		maploader::OnMapLoad(map);
		if (!maploader::GetBaseMapName(map, (const char**)&map)) {
			*g_PhasorLog << "maploader : unable to determine base map for " 
				<< map << endl;
		}
#endif

		current_map_base = map;
		return bMapUnchanged;

//		return game::maps::OnMapLoad(mapData);
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