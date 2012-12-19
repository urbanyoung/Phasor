#include "Game.h"
#include "../Server/ServerStreams.h"
#include "../Server/MapLoader.h"
#include "../Player.h"
#include <vector>

namespace halo { namespace game {
	//typedef std::unique_ptr<s_player> s_player_ptr;
	//s_player_ptr PlayerList[16];
	
	// Called when a game stage ends
	//void __stdcall OnGameEnd(DWORD mode)
	//{
	//
	//}

	// Called when a new game starts
	void OnNewGame(const char* map)
	{
		
	}

	// Called when a player joins (after verification).
	void __stdcall OnPlayerWelcome(DWORD playerId)
	{
	//	PlayerList[playerId] = s_player_ptr(new s_player(playerId));
		

	}

	// Called when a player quits
	void __stdcall OnPlayerQuit(DWORD playerId)
	{
		
	}
}}