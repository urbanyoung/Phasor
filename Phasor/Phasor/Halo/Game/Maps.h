#pragma once

#include <string>

namespace halo { namespace game {

	bool IsValidGametype(const std::wstring& gametype);
	bool LoadGametypes();
	const char* GetCurrentMap();
}}