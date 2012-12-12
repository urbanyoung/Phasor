#pragma once

#include "../../../Common/Types.h"
#include "../../../Common/Streams.h"

namespace halo { namespace game {

#ifdef PHASOR_PC
	// Returns the address of the loading buffer Halo should use
	char* GetLoadingMapBuffer();

	// Generates the map list
	void BuildMapList(COutStream& out);

	// Called when a map is being loaded
	bool OnMapLoad(LPBYTE mapData);

	// Called to fix the loaded map name (call when game begins)
	void FixMapName();

	// Updates the data in 'map' to the maps base data
	void UpdateLoadingMap(char* map);

	// This function returns the address of our map table
	DWORD GetMapTable();
#endif

	// This function checks if a map exists
	bool ValidateMap(char* map);
}}