#pragma once

#include "../../../Common/Types.h"
#pragma once

#include <string>

namespace halo { namespace server { namespace scriptloader
{	
	void LoadScripts();
	bool IsValidScript(const std::string& script);
}}}