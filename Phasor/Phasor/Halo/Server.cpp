#include "Server.h"
#include "Game/Maps.h"

namespace halo { namespace server
{
	// Called when a map is being loaded
	bool __stdcall OnMapLoad(BYTE* mapData)
	{
		return game::maps::OnMapLoad(mapData);
	}

}}