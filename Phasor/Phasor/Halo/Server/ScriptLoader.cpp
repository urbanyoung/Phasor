#include "ScriptLoader.h"
#include "../../../Scripting.h"
#include "MapLoader.h"

namespace halo { namespace server { namespace scriptloader
{
	void LoadScripts()
	{
		g_Scripts->CloseAllScripts();
		maploader::s_mapcycle_entry* entry = maploader::GetCurrentMapcycleEntry();

		if (entry) {
			for (size_t x = 0; x < entry->scripts->count; x++) {
				g_Scripts->OpenScript(entry->scripts->script_names[x]);
			}			
		}
	}

	bool IsValidScript(const std::string& script)
	{

	}
}}}