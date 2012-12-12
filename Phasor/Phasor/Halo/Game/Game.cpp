#include "Game.h"
#include "Maps.h"
#include "MapLoader.h"

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
		maploader::OnNewGame();
		map = maps::GetCurrentMapBaseName();
#endif
	}
}}