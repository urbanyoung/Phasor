#pragma once

#include <string>
#include "../../../Common/Types.h"
namespace halo { namespace game { namespace maps {

	bool IsValidGametype(const std::wstring& gametype);
	bool LoadGametypes();
	const char* GetCurrentMapBaseName();

	// Codecaves
	// Called when a map is being loaded
	bool OnMapLoad(BYTE* mapData);
}}}