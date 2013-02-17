#include "Server.h"
#include "../../../Common/MyString.h"
#include "../../Logging.h"
#include "ScriptLoader.h"
#include "MapLoader.h"
#include "../../Commands.h"
#include "../Game/Game.h"
#include "../../Globals.h"
#include "../../Admin.h"
#include "../../../ScriptingEvents.h"

#include "Packet.h"
#include "Chat.h"

namespace halo { namespace server
{
	SayStream say_stream;

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

/*	void __stdcall ConsoleHandler(DWORD fdwCtrlType)
	{

	}*/

	// Called periodically by Halo to check for console input, I use for timers
	void __stdcall OnConsoleProcessing()
	{
		g_Timers.Process();
		g_Thread.ProcessEvents();
	}

	/*! \todo make this more efficient.. shouldn't need to GetPlayerFromAddress */
	void __stdcall OnClientUpdate(s_player_structure* m_player)
	{
		s_player* player = game::GetPlayerFromAddress(m_player);

		if (player)	{
			game::OnClientUpdate(*player);
			scripting::events::OnClientUpdate(*player);
		}
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
		scriptloader::LoadScripts();
		game::OnNewGame(map);
	}

	// Called when a game stage ends
	void __stdcall OnGameEnd(DWORD mode)
	{
		game::OnGameEnd(mode);
	}

	void __stdcall OnHaloPrint(char* msg)
	{
		*g_PrintStream << msg << endl;
		s_player* player = GetPlayerExecutingCommand();
		if (player) *player->console_stream << msg << endl;
	}

	bool __stdcall OnHaloBanCheck(char* hash, s_machine_info* machine)
	{
		// fix an exploit where hashes can be uppercase (gamespy still validates
		// them but they are treated as different by the server)
		for (size_t i = 0; i < strlen(hash); i++)
			if (hash[i] >= 'A' && hash[i] <= 'Z')
				hash[i] += 32;
		
		std::string ip;
		GetMachineIP(*machine, &ip, NULL);
		
		bool allow = scripting::events::OnBanCheck(hash, ip);

		if (!allow) { 
			*g_PhasorLog << "Scripts rejected player. Hash " << hash << 
				" IP : " << ip << endl;
		}
		return allow;
	}

	// Called when a console command is to be executed
	// kProcessed: Event has been handled, don't pass to server
	// kGiveToHalo: Not handled, pass to server.
	e_command_result __stdcall ProcessCommand(char* command)
	{
		s_player* exec_player = GetPlayerExecutingCommand();
		bool can_execute = exec_player == NULL;

		if (can_execute) { // server console executing
			// save the command for memory (arrow keys)
			s_command_cache* cache = (s_command_cache*)ADDR_CMDCACHE;
			cache->count = (cache->count + 1) % 8;
			strcpy_s(cache->commands[cache->count], sizeof(cache->commands[cache->count]),
				command);
			cache->cur = 0xFFFF;
		} else {
			TempForwarder echo(*g_PrintStream, 
				TempForwarder::end_point(*g_RconLog));

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
						L"' Hash: " << exec_player->hash << " Authed as: '" 
						<< authName << "'" << endl;
					echo << "Command: '" << command << "'" << endl;
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

			if (!can_execute) *(exec_player->console_stream) << L" ** Access denied **" << endl;
		}

		COutStream& outStream = (exec_player == NULL) ? 
								static_cast<COutStream&>(*g_PrintStream) : 
								static_cast<COutStream&>(*exec_player->console_stream);
		
		return	can_execute ? 
				commands::ProcessCommand(command, outStream, exec_player) :
				e_command_result::kProcessed;
	}

	// --------------------------------------------------------------------
	// 
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

	void MessagePlayer(const s_player& player, const std::wstring& str)
	{
		if (str.size() > 150) return;
		chat::DispatchChat(chat::kChatServer, str.c_str(), NULL, &player);
	}

	bool ConsoleMessagePlayer(const s_player& player, const std::wstring& str)
	{
		if (str.size() >= 0x50) return false;

#pragma pack(push, 1)
		struct s_console_msg
		{
			char* msg_ptr;
			DWORD unk; // always 0
			char msg[0x50];

			s_console_msg(const char* text)
			{
				memset(msg, 0, 0x50);
				strcpy_s(msg, 0x50, text);
				unk = 0;
				msg_ptr = msg;
			}
		};
#pragma pack(pop)

		std::string str_narrow = NarrowString(str);
		s_console_msg d(str_narrow.c_str());
		/*static */BYTE buffer[8192]; 

#ifdef PHASOR_PC
		DWORD size = server::BuildPacket(buffer, 0, 0x37, 0, (LPBYTE)&d, 0,1,0);
#elif  PHASOR_CE 
		DWORD size = server::BuildPacket(buffer, 0, 0x38, 0, (LPBYTE)&d, 0,1,0);
#endif
		AddPacketToPlayerQueue(player.mem->playerNum, buffer, size, 1,1,0,1,3);
		
		return true;
	}

	void NotifyServerOfTeamChange(const halo::s_player& player)
	{
		// build the packet that notifies the server of the team change
		BYTE d[4] = {(BYTE)player.memory_id, (BYTE)player.mem->team, 0x18, 0};
		// Gotta pass a pointer to the data
		DWORD d_ptr = (DWORD)&d;		BYTE buffer[8192];		DWORD retval = server::BuildPacket(buffer, 0, 0x1A, 0, (LPBYTE)&d_ptr, 0, 1, 0);
		server::AddPacketToGlobalQueue(buffer, retval, 1, 1, 0, 1, 3);
	}

	void ExecuteServerCommand(const std::string& command)
	{
		halo::s_player* old_exec_player = GetPlayerExecutingCommand();
		const char* cmd = command.c_str();
		__asm
		{
			pushad
			PUSH 0x2000
			mov edi, cmd
			call dword ptr ds: [FUNC_EXECUTESVCMD]
			add esp, 4		
			popad
		}

		SetExecutingPlayer(old_exec_player);
	}

	void SetExecutingPlayer(halo::s_player* player)
	{
		if (!player) *(DWORD*)UlongToPtr(ADDR_RCONPLAYER) = -1;
		else *(DWORD*)UlongToPtr(ADDR_RCONPLAYER) = player->mem->playerNum;
	}

	halo::s_player* GetPlayerExecutingCommand()
	{
		DWORD execPlayerNumber = *(DWORD*)UlongToPtr(ADDR_RCONPLAYER);
		return game::GetPlayerFromRconId(execPlayerNumber);
	}

	bool GetPlayerHash(const s_player& player, std::string& hash)
	{
		s_machine_info* machine = GetMachineData(player);
		if (!machine) return false;
		return GetMachineHash(*machine, hash);
	}

	bool GetMachineHash(s_machine_info& machine, std::string& hash)
	{
		s_hash_list* hash_list = (s_hash_list*)ADDR_HASHLIST;
		hash_list = hash_list->next;
		while (hash_list && hash_list->data) {	
			if (hash_list->data->id == machine.id_hash) {
				hash = hash_list->data->hash;
				return true;
			}
			hash_list = hash_list->next;
		}
		return false;
	}
	
	bool GetPlayerIP(const s_player& player, std::string* ip, WORD* port)
	{
		s_machine_info* machine = GetMachineData(player);
		if (!machine) return false;
		return GetMachineIP(*machine, ip, port);
	}

	bool GetMachineIP(s_machine_info& machine, std::string* ip, WORD* port)
	{
		s_connection_info* con = machine.get_con_info();
		if (!con) return false;
		if (ip) {
			BYTE* ip_data = con->ip;
			*ip = m_sprintf("%d.%d.%d.%d", ip_data[0], ip_data[1],
				ip_data[2], ip_data[3]);
		}
		if (port) *port = con->port;

		return true;
	}
	
	// --------------------------------------------------------------------
	// Server message.
	bool SayStream::Write(const std::wstring& str)
	{
		std::wstring msg = L"** SERVER ** " + StripTrailingEndl(str);
		chat::DispatchChat(chat::kChatServer, msg.c_str());
		return true;
	}
}}