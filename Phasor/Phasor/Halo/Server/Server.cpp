#include "Server.h"
#include "../../../Common/MyString.h"
#include "../../Logging.h"
#include "ScriptLoader.h"
#include "MapLoader.h"
#include "../../Commands.h"
#include "../Game/Game.h"
#include "../../Globals.h"
#include "../../Admin.h"
#include "../../../Scripts/script-events.h"
#include "Mapvote.h"
#include "Packet.h"
#include "Chat.h"
#include "../../../Main.h"

namespace halo { namespace server
{
	SayStream say_stream;
	SayStreamRaw say_stream_raw;

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

	std::unique_ptr<PhasorMachine> machine_list[16];
	std::string current_map;

	// --------------------------------------------------------------------
	PhasorMachine::PhasorMachine(s_machine_info* machine)
		: machine(machine), hash_validated(false)
	{
		//*g_PrintStream << "Machine " << machine->machineNum << " is connecting.." << endl;
	}

	PhasorMachine::~PhasorMachine()
	{
		//*g_PrintStream << "Machine " << machine->machineNum << " has disconnected." << endl;
	}


	s_server_info* GetServerStruct()
	{
		return (s_server_info*)ADDR_SERVERSTRUCT; 
	}

	PhasorMachine* FindMachineById(DWORD machineNum)
	{
		for (int i = 0; i < 16; i++) {
			if (machine_list[i] && machine_list[i]->machine->machineNum == machineNum)
				return machine_list[i].get();
		}
		return NULL;
	}

	PhasorMachine* FindMachineByIndex(DWORD index)
	{
		if (index < 16 && index >= 0 && machine_list[index]) return machine_list[index].get();
		return NULL;
	}

	PhasorMachine* FindMachine(const s_player& player)
	{
		for (int i = 0; i < 16; i++) {
			if (machine_list[i] && machine_list[i]->machine->playerNum == player.mem->client_stuff.machineId)
				return machine_list[i].get();
		}
		return NULL;
	}

	void __stdcall OnMachineConnect(DWORD machineIndex)
	{
		s_server_info* server = GetServerStruct();
		s_machine_info* machine = &server->machine_table[machineIndex];
		machine_list[machineIndex].reset(new PhasorMachine(machine));
	}

	void __stdcall OnMachineDisconnect(DWORD machineIndex)
	{
		machine_list[machineIndex].reset();
	}

	void __stdcall OnMachineInfoFix(s_machinfo_info_partial_packet* data)
	{
		// ensure they're null terminated (halo is retarded)
		data->clientKey[8] = '\0';
		data->name[11] = L'\0';

		// now let scripts change the player's name
		std::string hash(data->hash, 32);
		std::string name = NarrowString(data->name);
		boost::optional<std::string> newName;

		bool allow_name = scripting::events::OnNameRequest(hash, name, newName);

		if (newName) {
			std::wstring wname = WidenString(*newName).substr(0, 11);
			wcscpy_s(data->name, 12, wname.c_str());
		} else if (!allow_name) {
			data->name[0] = L'\0';
		}
	}

	void __stdcall ConsoleHandler(DWORD fdwCtrlType)
	{
		switch(fdwCtrlType) 
		{ 
		case CTRL_CLOSE_EVENT: 
		case CTRL_SHUTDOWN_EVENT: 
		case CTRL_LOGOFF_EVENT: 
			{
				// cleanup everything
				OnServerClose();

			} break;
		}
	}

	// Called periodically by Halo to check for console input, I use for timers
	void __stdcall OnConsoleProcessing()
	{
		g_Timers.Process();
		g_Thread.ProcessEvents();
	}

	/*! \todo make this more efficient.. shouldn't need to GetPlayerFromAddress */
	void __stdcall OnClientUpdate(s_player_structure* m_player)
	{
		s_player* player = game::getPlayerFromAddress(m_player);

		if (player)	{
			game::OnClientUpdate(*player);
			scripting::events::OnClientUpdate(*player);
		}
	}

	// Called when a map is being loaded
	bool __stdcall OnMapLoad(maploader::s_mapcycle_entry* loading_map)
	{
		bool bMapUnchanged = true;

		maploader::s_phasor_mapcycle_entry vote_decision;
		if (mapvote::GetVoteDecision(vote_decision))
		{
			if (!maploader::ReplaceHaloMapEntry(loading_map,
				vote_decision, *g_PhasorLog))
			{
				*g_PhasorLog << "Map vote ignored due to previous error." << endl;
			}
		}

		char* map = loading_map->map;
		current_map = map;
		//char* gametype = loading_map->gametype;
#ifdef PHASOR_PC		
		maploader::OnMapLoad(map);
		if (!maploader::GetBaseMapName(map, (const char**)&map)) {
			*g_PhasorLog << "maploader : unable to determine base map for " 
				<< map << endl;
		}
#endif

		return bMapUnchanged;
	}

	// Called when a new game starts
	void __stdcall OnNewGame(const char* map)
	{
#ifdef PHASOR_PC
		// Fix the map name
		maploader::OnNewGame();
#endif
		// from previous game
		game::cleanupPlayers(true);

		scriptloader::LoadScripts();
		game::OnNewGame(map);
	}

	// Called when a game stage ends
	void __stdcall OnGameEnd(DWORD mode)
	{
		game::OnGameEnd(mode);

		if (mode == 3) {
			mapvote::BeginVote();
		}
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

	
	bool allow_invalid_hash = false;
	void acceptInvalidHashes(bool state) { allow_invalid_hash = state; }
	bool getInvalidHashState() { return allow_invalid_hash; }

	// Called once Halo has received the hash-checking response from gamespy
	void __stdcall OnHashValidation(s_hash_validation* info, const char* status)
	{
		// We still want to reject valid hashes with invalid challenges
		// If we get such a case, someone is trying to steal a hash.
		if (allow_invalid_hash && strcmp(status, "Invalid authentication") != 0) info->status = 1;

		if (info->status == 1) {
			PhasorMachine* machine = FindMachineById(info->machineId);
			machine->hash_validated = true;
			
			s_player* player = game::getPlayerFromHash(info->hash);
			if (player) player->checkAndSetAdmin();
		} else {
			*g_PrintStream << "Machine " << info->machineId << " is being rejected because: " << status << endl;
		}
	}

	void __stdcall ProcessCommandAttempt(s_command_input* input, int playerNum)
	{
		// Make sure strings are null terminated
		input->command[sizeof(input->command)-1] = 0;
		input->password[sizeof(input->password)-1] = 0;

		if (input->command[0] == 0) return; // empty command

		const char* password = (const char*)ADDR_RCONPASSWORD;
		bool bCorrect = !strcmp(password, input->password);

		s_player* player = game::getPlayerFromRconId(playerNum);

		// Asks scripts if we should allow it.
		if (!bCorrect) 
			bCorrect = scripting::events::OnServerCommandAttempt(*player, input->command, input->password);

		if (bCorrect) {			
			SetExecutingPlayer(player);
			ExecuteServerCommand(input->command, player);
			SetExecutingPlayer(NULL);
		}
	}

	// Called when a console command is to be executed
	// kProcessed: Event has been handled, don't pass to server
	// kGiveToHalo: Not handled, pass to server.
	e_command_result __stdcall ProcessCommand(char* cmd)
	{
		s_player* exec_player = GetPlayerExecutingCommand();
		bool can_execute = exec_player == NULL;

		std::string command = cmd;

		std::vector<std::string> tokens = TokenizeArgs(command);
		if (!tokens.size()) return e_command_result::kProcessed; 

		if (!can_execute) {
			TempForwarder echo(*g_PrintStream, 
				TempForwarder::end_point(*g_RconLog));

			if (exec_player->authenticating_hash) {
				*exec_player->console_stream << "Please wait while your hash is authenticated.." << endl;
			} else {
				// Find just the command name
				std::string commandName = command.substr(0, command.find(' '));

				std::string authName;
				Admin::result_t result = Admin::canUseCommand(exec_player->hash,
					commandName, &authName);

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
						echo << "Command: '" << commandName << "'" << endl;
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
		}

		e_command_result result = e_command_result::kProcessed;
		if (can_execute) {
			COutStream& outStream = (exec_player == NULL) ? 
					static_cast<COutStream&>(*g_PrintStream) : 
					static_cast<COutStream&>(*exec_player->console_stream);

			result = commands::ProcessCommand(command, outStream, exec_player);
		}

		if (exec_player == NULL && result == e_command_result::kProcessed) {
			// save the command for memory (arrow keys)
			s_command_cache* cache = (s_command_cache*)ADDR_CMDCACHE;
			cache->count = (cache->count + 1) % 8;
			strcpy_s(cache->commands[cache->count], sizeof(cache->commands[cache->count]),
				cmd);
			cache->cur = 0xFFFF;
		}
		return result;
	}

	// --------------------------------------------------------------------
	//

	void MessagePlayer(const s_player& player, const std::wstring& str)
	{
		if (str.size() > 150) return;
		chat::e_chat_types type = chat::e_chat_types::kChatPrivate;
		std::string change_msg;
		bool allow = scripting::events::OnServerChat(&player, NarrowString(str),
			type, change_msg);

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
		AddPacketToPlayerQueue(player.mem->client_stuff.machineId, buffer, size, 1,1,0,1,3);
		
		return true;
	}

	void NotifyServerOfTeamChange(const halo::s_player& player)
	{
		// build the packet that notifies the server of the team change
		BYTE d[4] = {(BYTE)player.memory_id, (BYTE)player.mem->team, 0x18, 0};

		// Gotta pass a pointer to the data
		DWORD d_ptr = (DWORD)&d;
		BYTE buffer[8192];
		DWORD retval = server::BuildPacket(buffer, 0, 0x1A, 0, (LPBYTE)&d_ptr, 0, 1, 0);
		server::AddPacketToGlobalQueue(buffer, retval, 1, 1, 0, 1, 3);
	}

	void ExecuteServerCommand(const std::string& command, s_player* execute_as)
	{
		halo::s_player* old_exec_player = GetPlayerExecutingCommand();
		SetExecutingPlayer(execute_as);

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
		else *(DWORD*)UlongToPtr(ADDR_RCONPLAYER) = player->mem->client_stuff.machineId;
	}

	halo::s_player* GetPlayerExecutingCommand()
	{
		DWORD execPlayerNumber = *(DWORD*)UlongToPtr(ADDR_RCONPLAYER);
		return game::getPlayerFromRconId(execPlayerNumber);
	}

	bool GetPlayerHash(const s_player& player, std::string& hash)
	{
		PhasorMachine* machine = FindMachine(player);
		if (!machine) return false;
		return GetMachineHash(*machine->machine, hash);
	}

	bool GetMachineHash(const s_machine_info& machine, std::string& hash)
	{
		s_hash_list* hash_list = (s_hash_list*)ADDR_HASHLIST;
		hash_list = hash_list->next;
		while (hash_list && hash_list->data) {	
			if (hash_list->data->id == machine.machineNum) {
				hash = hash_list->data->hash;
				return true;
			}
			hash_list = hash_list->next;
		}
		return false;
	}
	
	bool GetPlayerIP(const s_player& player, std::string* ip, WORD* port)
	{
		PhasorMachine* machine = FindMachine(player);
		if (!machine) return false;
		return GetMachineIP(*machine->machine, ip, port);
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

	/*! \todo parse these structures */
	DWORD GetRespawnTicks()
	{
		return *(DWORD*)(ADDR_GAMETYPE + OFFSET_RESPAWNTICKS);
	}

	DWORD GetServerTicks()
	{
		DWORD server_base = *(DWORD*)ADDR_SERVERINFO;
		DWORD ticks = *(DWORD*)(server_base + 0x0C);
		return ticks;
	}

	e_command_result sv_quit(void*, 
		commands::CArgParser&, COutStream&)
	{
		OnServerClose();
		return e_command_result::kProcessed;
	}
	
	// --------------------------------------------------------------------
	// Server message.
	// 
	bool SayStreamRaw::Write(const std::wstring& str)
	{
		chat::e_chat_types type = chat::e_chat_types::kChatServer;
		std::string change;
		bool allow = scripting::events::OnServerChat(NULL, NarrowString(str),
			type, change);
		if (allow) {
			std::wstring msg = str;
			if (change.size()) msg = WidenString(change);
			chat::DispatchChat(type, msg.c_str());
		}
		return true;
	}

	bool SayStream::Write(const std::wstring& str)
	{
		std::wstring msg = MSG_PREFIX + StripTrailingEndl(str);
		return SayStreamRaw::Write(msg);
	}
}}