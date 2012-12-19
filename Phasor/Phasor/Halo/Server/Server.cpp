#include "Server.h"
#include "../../../Common/MyString.h"
#include "../../Logging.h"
#include "ScriptLoader.h"
#include "MapLoader.h"
#include "../../Commands.h"
#include "../Game/Game.h"
#include "../../Globals.h"

namespace halo { namespace server
{
	std::string current_map_base;

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

	// Called when a new game starts
	void __stdcall OnNewGame(const char* map)
	{
#ifdef PHASOR_PC
		// Fix the map name
		maploader::OnNewGame();
#endif
		game::OnNewGame(map);
		scriptloader::LoadScripts();
	}

	// Called when a console command is to be executed
	// kProcessed: Event has been handled, don't pass to server
	// kGiveToHalo: Not handled, pass to server.
	e_command_result __stdcall ProcessCommand(char* command)
	{
		CHaloEchoStream echo(*g_RconLog);
		//echo
		//Admin::result_t result = Admin::CanUseCommand()
		// do admin checks here
		// call scripts etc
		// if executing from console use g_PrintStream else g_RconLog
		return commands::ProcessCommand(std::string(command), g_PrintStream);
		/*std::vector<std::string> args = TokenizeArgs(command);
		if (args.size() == 0) return e_command_result::kProcessed; // nothing to process
		
		for (size_t x = 0; cmd_tbl[x].key != NULL; x++) {
			if (cmd_tbl[x].key == args[0]) {
				return cmd_tbl[x].fn(NULL, args, g_PrintStream);
			}
		}

		return e_command_result::kGiveToHalo;*/
	}

	// This function is effectively sv_map_next
	void StartGame(const char* map)
	{
		if (*(DWORD*)ADDR_GAMEREADY != 2) {
			// start the server not just game
			__asm
			{
				pushad
				MOV EDI,map
				CALL dword ptr ds:[FUNC_PREPAREGAME_ONE]
				push 0
				push esi // we need a register for a bit
				mov esi, dword ptr DS:[ADDR_PREPAREGAME_FLAG]
				mov byte PTR ds:[esi], 1
				pop esi
				call dword ptr ds:[FUNC_PREPAREGAME_TWO]
				add esp, 4
				popad
			}
		}
		else {
			// Halo 1.09 addresses
			// 00517845  |.  BF 90446900   MOV EDI,haloded.00694490                                  ;  UNICODE "ctf1"
			//0051784A  |.  F3:A5         REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS:[ESI]
			//0051784C  |.  6A 00         PUSH 0
			//	0051784E  |.  C605 28456900>MOV BYTE PTR DS:[694528],1
			//	00517855  |.  E8 961B0000   CALL haloded.005193F0                                     ;  start server
			__asm
			{
				pushad
				call dword ptr ds:[FUNC_EXECUTEGAME]
				popad
			}
		}
	}

	void MessageAllPlayers(const wchar_t* fmt, ...)
	{
		va_list ArgList;
		va_start(ArgList, fmt);
		std::wstring str = FormatVarArgsW(fmt, ArgList);
		va_end(ArgList);

		g_PrintStream << "todo: make this message all players - " << str << endl;
	}
}}