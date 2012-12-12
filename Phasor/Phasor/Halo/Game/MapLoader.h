#pragma once

#include "../../../Common/Types.h"
#include "../../../Common/Streams.h"

namespace halo { namespace game { namespace maploader {

#ifdef PHASOR_PC
	// Returns the address of the loading buffer Halo should use
	char* GetLoadingMapBuffer();

	// Generates the map list
	void BuildMapList(COutStream& out);

	// This function returns the address of our map table
	DWORD GetMapTable();

	// This function checks if a map exists
	bool ValidateMap(char* map);

	// Called when a map is being loaded.
	void OnMapLoad(char* map);

	// Called to fix the loaded map name (call when game begins)
	void OnNewGame();
#endif	
}}}