#include "Server.h"
#include "../../../Common/MyString.h"
#include "../../Logging.h"
#include "ScriptLoader.h"
#include "MapLoader.h"
#include "../../Commands.h"
#include "../Game/Game.h"
#include "../../Globals.h"
#include "../../Admin.h"

namespace halo { namespace server
{
	#pragma pack(push, 1)
	struct s_hash_data
	{
		DWORD id;
		char hash[0x20];
		BYTE unk[0x2c];
	};
	static_assert(sizeof(s_hash_data) == 0x50, "incorrect s_hash_data");

	struct s_hash_list
	{
		s_hash_data* data; // 0 for head of list
		s_hash_list* next;
		s_hash_list* prev; 
	};
	static_assert(sizeof(s_hash_list) == 0x0C, "incorrect s_hash_list");

	struct s_command_cache
	{
		char commands[8][0xFF];
		WORD unk;
		WORD count;
		WORD cur;
	};
	#pragma pack(pop)
	std::string current_map_base;

	s_server_info* GetServerStruct()
	{
		return (s_server_info*)ADDR_SERVERSTRUCT; 
	}

	s_machine_info* GetMachineData(const s_player& player)
	{		
		s_server_info* server = GetServerStruct();
		for (int i = 0;i < 16; i++) {
			if (server->machine_table[i].playerNum == player.mem->playerNum)
				return &server->machine_table[i];
		}
		return NULL;
	}

	// Called periodically by Halo to check for console input, I use for timers
	void __stdcall OnConsoleProcessing()
	{
		g_Timers.Process();
	}

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

	// Called when a game stage ends
	void __stdcall OnGameEnd(DWORD mode)
	{
		game::OnGameEnd(mode);
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
		std::unique_ptr<COutStream> echo_ptr;

		s_player* exec_player = GetExecutingPlayer();
		bool can_execute = exec_player == NULL;
		// create the output stream
		if (can_execute) { // server console executing
			echo_ptr.reset(new CHaloPrintStream());
			// save the command for memory (arrow keys)
			s_command_cache* cache = (s_command_cache*)ADDR_CMDCACHE;
			cache->count = (cache->count + 1) % 8;
			strcpy_s(cache->commands[cache->count], sizeof(cache->commands[cache->count]),
				command);
			cache->cur = 0xFFFF;
		} else {
			echo_ptr.reset(new CEchoStream(*exec_player->stream, *g_RconLog));
		}

		if (!can_execute) {
			// don't want the person executing it to see this output
			CEchoStream echo(g_PrintStream, *g_RconLog);
			std::string authName;
			Admin::result_t result = Admin::CanUseCommand(exec_player->hash,
				command, &authName);

			e_command_result do_process = e_command_result::kProcessed;
			switch (result)
			{
			case Admin::E_NOT_ADMIN:
				{
					echo << L"An unauthorized player is attempting to use RCON:" << endl;
					echo << L"Name: '" << exec_player->mem->playerName <<
						L"' Hash: " << exec_player->hash << endl;
				} break;
			case Admin::E_NOT_ALLOWED:
				{
					echo << L"An authorized player is attempting to use an unauthorized command:" << endl;
					echo << L"Name: '" << exec_player->mem->playerName <<
						L"' Hash: " << exec_player->hash << endl;
				} break;
			case Admin::E_OK:
				{
					can_execute = true;
					echo << L"Executing ' " << command << L" ' from " <<
						exec_player->mem->playerName;
					if (authName.size())
						echo << L" (authed as '" <<	authName << L"').";
					echo << endl;							
					
				} break;
			}

			if (!can_execute) *exec_player->stream << L" ** Access denied **" << endl;
		}

		return can_execute ? commands::ProcessCommand(command, *echo_ptr, exec_player)
			: e_command_result::kProcessed;
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

	void MessagePlayer(s_player& player, const std::wstring& str)
	{
		g_PrintStream << "todo: make this message the player - " << str << endl;
	}

	void MessageAllPlayers(const wchar_t* fmt, ...)
	{
		va_list ArgList;
		va_start(ArgList, fmt);
		std::wstring str = FormatVarArgsW(fmt, ArgList);
		va_end(ArgList);

		for (int i = 0; i < 16; i++) {
			s_player* player = game::GetPlayer(i);
			if (player) MessagePlayer(*player, str);
		}		
	}

	bool GetPlayerIP(const s_player& player, std::string* ip, WORD* port)
	{
		s_machine_info* machine = GetMachineData(player);
		if (!machine) return false;
		if (ip) {
			BYTE* ip_data = machine->get_con_info()->ip;
			*ip = m_sprintf("%d.%d.%d.%d", ip_data[0], ip_data[1],
				ip_data[2], ip_data[3]);
		}
		if (port) *port = machine->get_con_info()->port;

		return true;
	}

	bool GetPlayerHash(const s_player& player, std::string& hash)
	{
		s_machine_info* machine = GetMachineData(player);
		if (!machine) return false;
		s_hash_list* hash_list = (s_hash_list*)ADDR_HASHLIST;
		hash_list = hash_list->next;
		bool found = false;
		while (hash_list && hash_list->data) {	
			if (hash_list->data->id == machine->id_hash) {
				hash = hash_list->data->hash;
				found = true;
				break;
			}
			hash_list = hash_list->next;
		}
		return found;
	}
}}