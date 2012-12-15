#pragma once

#include "../../../Common/Types.h"
#include "../../../Common/Streams.h"

namespace halo { namespace server { namespace maploader {

#ifdef PHASOR_PC
	// Returns the address of the loading buffer Halo should use
	char* GetLoadingMapBuffer();

	// Generates the map list
	void BuildMapList(COutStream& out);

	// This function returns the address of our map table
	DWORD GetMapTable();

	// Checks if a map exists
	bool ValidateMap(char* map);

	// Called when a map is being loaded.
	void OnMapLoad(char* map);

	// Called to fix the loaded map name
	void OnNewGame();

	// Returns the base name for a map (ie bloodgulch1 -> bloodgulch)
	bool GetBaseMapName(const char* actual_map, const char** out);
#endif	

	#pragma pack(push, 1)
	struct s_mapcycle_entry
	{
		char* map;
		char* gametype;
		//char* script;
	};
	#pragma pack(pop)
}}}