#pragma once

#include <string>
#include "../../../Common/Types.h"

namespace halo { namespace game { namespace gametypes {

	bool IsValidGametype(const std::wstring& gametype);
	bool BuildGametypeList();
	bool ReadGametypeData(const std::wstring& gametypePath, BYTE* out,
		DWORD outSize);

}}}