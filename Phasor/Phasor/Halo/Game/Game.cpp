#include "Game.h"
#include "../Server/ServerStreams.h"
#include "../Server/MapLoader.h"
#include "../Player.h"
#include "../../Globals.h"
#include "../../../Common/MyString.h"
#include <vector>
namespace halo { namespace game {
	typedef std::unique_ptr<s_player> s_player_ptr;
	s_player_ptr PlayerList[16];

	inline bool valid_index(DWORD playerIndex)
	{
		return playerIndex >= 0 && playerIndex < 16;
	}

	s_player* GetPlayer(int index)
	{
		if (!valid_index(index)) return NULL;
		return PlayerList[index] ? PlayerList[index].get() : NULL;
	}

	s_player* GetPlayerFromRconId(int playerNum)
	{
		for (int i = 0; i < 16; i++) {
			if (PlayerList[i] && PlayerList[i]->mem->playerNum == playerNum)
				return PlayerList[i].get();
		}
		return NULL;
	}
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
		if (!valid_index(playerId)) {
			*g_PhasorLog << "Player joined with invalid index??" << endl;
			return;
		}
		PlayerList[playerId].reset(new s_player(playerId));
		s_player* player = GetPlayer(playerId);	
		g_GameLog->WriteLog(kPlayerJoin, L"%s (%s ip: %s:%i)", 
			player->mem->playerName, WidenString(player->hash).c_str(),
			WidenString(player->ip).c_str(), player->port);

	}

	// Called when a player quits
	void __stdcall OnPlayerQuit(DWORD playerId)
	{
		if (!valid_index(playerId)) {
			*g_PhasorLog << "Player left with invalid index??" << endl;
			return;
		}
		s_player* player = GetPlayer(playerId);

		if (player) {
			
			g_GameLog->WriteLog(kPlayerLeave, L"%s (%s)", 
				player->mem->playerName, WidenString(player->hash).c_str()
				);

			PlayerList[playerId].reset();
		}
	}
}}