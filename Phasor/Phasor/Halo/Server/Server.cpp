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

	s_player* GetExecutingPlayer()
	{
		int playerNum = *(int*)UlongToPtr(ADDR_RCONPLAYER);
		return game::GetPlayerFromRconId(playerNum);
	}

	// Called when a console command is to be executed
	// kProcessed: Event has been handled, don't pass to server
	// kGiveToHalo: Not handled, pass to server.
	e_command_result __stdcall ProcessCommand(char* command)
	{
		CHaloEchoStream echo(*g_RconLog);

		s_player* exec_player = GetExecutingPlayer();
		if (exec_player) {
			if (!exec_player->IsAdmin()) {
				g_PrintStream << "Not admin" << endl;
			}
		}
		//echo
		//Admin::result_t result = Admin::CanUseCommand()
		// do admin checks here
		// call scripts etc
		// if executing from console use g_PrintStream else g_RconLog
		return commands::ProcessCommand(command, g_PrintStream, exec_player);
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

	#pragma pack(push, 1)
	struct s_player_ip
	{
		BYTE ip[4];
		WORD port;
	};
	#pragma pack(pop)

	bool GetPlayerIP(s_player& player, std::string* ip, WORD* port)
	{
		LPBYTE serverBase = (LPBYTE)*(DWORD*)*(DWORD*)ADDR_PLAYERINFOBASE;
		LPBYTE machineTable = serverBase + 0xAA0;
		LPBYTE machine = (LPBYTE)*(DWORD*)(machineTable + 4*player.mem->playerNum);

		if (!machine) false;
		LPBYTE pMachine = (LPBYTE)*(DWORD*)machine;
		if (!pMachine) false;
		
		s_player_ip* network = (s_player_ip*)*(DWORD*)pMachine;
		if (!network) return false;

		if (ip) {
			*ip = m_sprintf("%d.%d.%d.%d", network->ip[0], network->ip[1],
				network->ip[2], network->ip[3]);
		}
		if (port) *port = network->port;
		return true;
	}

	// todo: change how i get the hash
	int GetHashOffset(s_player& player)
	{
		LPBYTE origin = (LPBYTE)ADDR_PLAYERINFOBASE;
		LPBYTE lpBase = (LPBYTE)UlongToPtr(OFFSET_HASHBASE + *(DWORD*)origin);

		for (int i = 0; i < 16; i++)
		{
			WORD check = *(WORD*)lpBase;

			if (check == player.mem->playerNum)
				return *(int*)(lpBase + 0x50);

			lpBase += OFFSET_HASHLOOKUPLEN;
		}

		return -1;
	}

	bool GetPlayerHash(s_player& player, std::string& hash)
	{
		int offset = GetHashOffset(player);
		if (offset == -1) return false;
		typedef char* (__cdecl *_HaloGetHash)(DWORD type, DWORD offset);
		_HaloGetHash Halo_GetHash = (_HaloGetHash)(FUNC_HALOGETHASH);
		char* str_hash = Halo_GetHash(0x319, offset);
		if (str_hash) {
			hash = str_hash;
			if (hash.size() != 32) hash.clear();
		}
		return hash.size() == 32;
	}
}}