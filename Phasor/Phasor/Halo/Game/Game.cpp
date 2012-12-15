#include "Game.h"
#include "../Server/Common.h"
#include "../Server/MapLoader.h"

namespace halo { namespace game {
	// Called when a game stage ends
	//void __stdcall OnGameEnd(DWORD mode)
	//{
	//
	//}

	// Called when a new game starts
	void __stdcall OnNewGame(const char* map)
	{
#ifdef PHASOR_PC
		// Fix the map name
		server::maploader::OnNewGame();
#endif
	}
}}