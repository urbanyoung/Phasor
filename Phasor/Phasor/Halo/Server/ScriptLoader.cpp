#include "ScriptLoader.h"
#include "../../../Scripts/scripting.hpp"
#include "../../../Common/FileIO.h"
#include "../../../Common/MyString.h"
#include "../../Directory.h"
#include "MapLoader.h"
#include "../../Globals.h"

namespace halo { namespace server { namespace scriptloader
{
	void LoadScripts()
	{
		g_Scripts->unloadAllScripts(false);
		maploader::s_mapcycle_entry* entry = maploader::GetCurrentMapcycleEntry();

		if (entry && entry->scripts) {
			for (size_t x = 0; x < entry->scripts->count; x++) {
				*g_PrintStream << "Loading script " << entry->scripts->script_names[x] << endl;
				g_Scripts->loadScript(entry->scripts->script_names[x], false);
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

			if (g_Scripts->reloadScript(script))
				out << script << " has been reloaded." << endl;
			else
				out << script << " isn't currently loaded." << endl;
		} else {
			g_Scripts->reloadAllScripts(true);
			out << "All scripts have been reloaded." << endl;
		}
		return e_command_result::kProcessed;
	}

	e_command_result sv_script_load(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		std::string script = args.ReadString();
		bool persistent = false;
		if (args.size() == 2) persistent = args.ReadBool();
		
		if (g_Scripts->loadScript(script.c_str(), persistent))
			out << script << " has been loaded." << endl;
		else
			out << script << " couldn't be loaded." << endl;
		return e_command_result::kProcessed;
	}

	e_command_result sv_script_unload(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		g_Scripts->unloadScript(args.ReadString().c_str());
		out << "The script has been unloaded." << endl;
		return e_command_result::kProcessed;
	}

	e_command_result sv_script_list(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		out.wprint(L"%-40s%s", L"Script", L"Persistent");
		out.wprint(L"%s", L"--------------------------------------------------");
		auto loaded = g_Scripts->getLoadedScripts();
		for (auto itr = loaded.begin(); itr != loaded.end(); ++itr) {
			out.wprint(L"%-40s%i", WidenString(itr->first).c_str(), itr->second);
		}
		return e_command_result::kProcessed;
	}
}}}