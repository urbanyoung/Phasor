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
	void StartGame(const char* map);
	void MessageAllPlayers(const wchar_t* fmt, ...);
	bool GetPlayerIP(s_player& player, std::string* ip, WORD* port);
	bool GetPlayerHash(s_player& player, std::string& hash);

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

	// Called when a map is being loaded
	bool __stdcall OnMapLoad(maploader::s_mapcycle_entry* loading_map);

	// Called when the server (not Phasor) wants to print a message.
	void __stdcall OnHaloPrint(char* msg);

	// Called when halo checks a player's hash
	bool __stdcall OnHaloBanCheck(char* hash);

	// Called when the server info is about to be broadcast
	bool __stdcall OnVersionBroadcast(DWORD arg1, DWORD arg2);

} }