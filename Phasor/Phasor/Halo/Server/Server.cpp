#include "Server.h"
#include "../../../Common/MyString.h"
#include "Common.h"
#include "../../Logging.h"
#include "ScriptLoader.h"

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

	// This function is effectively sv_map_next
	void StartGame(const char* map)
	{
		if (*(DWORD*)ADDR_GAMEREADY != 2) {
			// start the server not just game
			DWORD dwInitGame1 = FUNC_PREPAREGAME_ONE;
			DWORD dwInitGame2 = FUNC_PREPAREGAME_TWO;
			__asm
			{
				pushad
				MOV EDI,map
				CALL dword ptr ds:[dwInitGame1]

				push 0
				push esi // we need a register for a bit
				mov esi, dword ptr DS:[ADDR_PREPAREGAME_FLAG]
				mov byte PTR ds:[esi], 1
				pop esi
				call dword ptr ds:[dwInitGame2]
				add esp, 4
				popad
			}
		}
		else
		{
			// Halo 1.09 addresses
			// 00517845  |.  BF 90446900   MOV EDI,haloded.00694490                                  ;  UNICODE "ctf1"
			//0051784A  |.  F3:A5         REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS:[ESI]
			//0051784C  |.  6A 00         PUSH 0
			//	0051784E  |.  C605 28456900>MOV BYTE PTR DS:[694528],1
			//	00517855  |.  E8 961B0000   CALL haloded.005193F0                                     ;  start server
			DWORD dwStartGame = FUNC_EXECUTEGAME;

			__asm
			{
				pushad
				call dword ptr ds:[dwStartGame]
				popad
			}
		}
	}
}}