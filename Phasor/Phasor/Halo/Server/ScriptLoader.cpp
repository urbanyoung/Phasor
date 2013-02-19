#include "ScriptLoader.h"
#include "../../../Scripting.h"
#include "../../../Common/FileIO.h"
#include "../../../Common/MyString.h"
#include "../../Directory.h"
#include "MapLoader.h"
#include "../../Globals.h"

namespace halo { namespace server { namespace scriptloader
{
	void LoadScripts()
	{
		g_Scripts->CloseAllScripts();
		maploader::s_mapcycle_entry* entry = maploader::GetCurrentMapcycleEntry();

		if (entry && entry->scripts) {
			for (size_t x = 0; x < entry->scripts->count; x++) {
				*g_PrintStream << "Opening script " << entry->scripts->script_names[x] << endl;
				g_Scripts->OpenScript(entry->scripts->script_names[x]);
			}			
		}
	}

	bool IsValidScript(const std::string& script)
	{
		std::wstring path = g_ScriptsDirectory + WidenString(script) + L".lua";
		return NDirectory::IsValidFile(path);		
	}

	e_command_result sv_script_reload(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		if (args.size() == 1) {
			std::string script = args.ReadString();

			if (g_Scripts->ReloadScript(script))
				out << script << " has been reloaded." << endl;
			else
				out << script << " isn't currently loaded." << endl;
		} else {
			g_Scripts->ReloadScripts();
			out << "All scripts have been reloaded." << endl;
		}
		return e_command_result::kProcessed;
	}

	e_command_result sv_script_load(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		std::string script = args.ReadString();
		
		if (g_Scripts->OpenScript(script.c_str()))
			out << script << " has been loaded." << endl;
		else
			out << script << " couldn't be loaded." << endl;
		return e_command_result::kProcessed;
	}
}}}