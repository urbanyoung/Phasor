#include "ScriptLoader.h"
#include "../../../Scripting.h"
#include "../../../Common/FileIO.h"
#include "../../../Common/MyString.h"
#include "../../Directory.h"
#include "MapLoader.h"

namespace halo { namespace server { namespace scriptloader
{
	void LoadScripts()
	{
		g_Scripts->CloseAllScripts();
		maploader::s_mapcycle_entry* entry = maploader::GetCurrentMapcycleEntry();

		if (entry && entry->scripts) {
			for (size_t x = 0; x < entry->scripts->count; x++) {
				g_Scripts->OpenScript(entry->scripts->script_names[x]);
			}			
		}
	}

	bool IsValidScript(const std::string& script)
	{
		std::wstring path = g_ScriptsDirectory + WidenString(script);
		return NDirectory::IsValidFile(path);		
	}
}}}