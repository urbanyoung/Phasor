#pragma once

#include "../../../Common/Types.h"
#include "MapLoader.h"

// stupid enum warning
#pragma warning( disable : 4482)
enum e_command_result
{
	kGiveToHalo = 0,
	kProcessed
};

namespace halo { 
	struct s_player;
namespace server
{
	#pragma pack(push, 1)
	struct s_connection_info
	{
		BYTE ip[4];
		WORD port;
		WORD pad;
		BYTE unk[0x148]; // handshake stuff etc, don't care atm.
	};
	static_assert(sizeof(s_connection_info) == 0x150, "incorrect s_connection_info");

	struct s_machine_info
	{
		s_connection_info*** con_info_ptr; 
		DWORD zero;
		DWORD zero1;
		WORD playerNum; // used for rcon etc
		WORD seven;
		BYTE unk[0x42];
		char key[10]; // only 7 chars long tho.. i think rest is padding
		DWORD id_hash;

		s_connection_info* get_con_info()
		{
			return **con_info_ptr;
		}
#ifdef PHASOR_CE
		char unk1[0x20]
		char ip[0x20]
		char cd_key_hash[0x20];
		char unk2[0x2c]
#endif
	};
	static_assert(sizeof(s_machine_info) == MACHINE_ENTRY_SIZE, "incorrect s_machine_info");

	struct s_presence_item
	{
		wchar_t name[12];
		DWORD idk;
		BYTE machineId;
		BYTE status;
		BYTE team;
		BYTE playerId;
	};
	static_assert(sizeof(s_presence_item) == 0x20, "incorrect s_presence_item");
	struct s_server_info
	{
		void* unk_ptr;
		WORD state; // 0 inactive, 1 game, 2 results
		WORD unk1;
		wchar_t server_name[0x42];
		char map_name[0x80];
		wchar_t gametype[0x18];
		BYTE unk2[0x69];
		BYTE max_players;
		WORD unk3;
		BYTE cur_players;
		BYTE unk4;
		s_presence_item player_data[16];
		BYTE unk5[14];
		s_machine_info machine_table[16];
	};
	#pragma pack(pop)

	void StartGame(const char* map);
	void MessagePlayer(s_player& player, const std::wstring& str);
	void MessageAllPlayers(const wchar_t* fmt, ...);
	bool GetPlayerIP(const s_player& player, std::string* ip, WORD* port);
	bool GetPlayerHash(const s_player& player, std::string& hash);
	s_machine_info* GetMachineData(const s_player& player);


	// --------------------------------------------------------------------
	// Events
	
	// Called for console events (exit etc)
	void __stdcall ConsoleHandler(DWORD fdwCtrlType);

	// Called periodically by Halo to check for console input, I use for timers
	void __stdcall OnConsoleProcessing();

	// Called when a console command is to be executed
	// true : Event has been handled, don't pass to server
	// false: Not handled, pass to server.
	e_command_result __stdcall ProcessCommand(char* command);

	void __stdcall OnNewGame(const char* map);
	void __stdcall OnGameEnd(DWORD mode);

	// Called when a map is being loaded
	bool __stdcall OnMapLoad(maploader::s_mapcycle_entry* loading_map);

	// Called when the server (not Phasor) wants to print a message.
	void __stdcall OnHaloPrint(char* msg);

	// Called when halo checks a player's hash
	bool __stdcall OnHaloBanCheck(char* hash);

	// Called when the server info is about to be broadcast
	bool __stdcall OnVersionBroadcast(DWORD arg1, DWORD arg2);

} }