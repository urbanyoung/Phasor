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
		g_Scripts->CloseAllScripts(false);
		maploader::s_mapcycle_entry* entry = maploader::GetCurrentMapcycleEntry();

		if (entry && entry->scripts) {
			for (size_t x = 0; x < entry->scripts->count; x++) {
				*g_PrintStream << "Opening script " << entry->scripts->script_names[x] << endl;
				g_Scripts->OpenScript(entry->scripts->script_names[x], false);
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
			g_Scripts->ReloadScripts(true);
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
		
		if (g_Scripts->OpenScript(script.c_str(), persistent))
			out << script << " has been loaded." << endl;
		else
			out << script << " couldn't be loaded." << endl;
		return e_command_result::kProcessed;
	}

	e_command_result sv_script_unload(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		g_Scripts->CloseScript(args.ReadString().c_str());
		return e_command_result::kProcessed;
	}

	e_command_result sv_script_list(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		out.wprint(L"%-20s%s", L"Script", L"Persistent");
		out.wprint(L"%s", L"------------------------------");
		auto loaded = g_Scripts->getLoadedScripts();
		for (auto itr = loaded.begin(); itr != loaded.end(); ++itr) {
			out.wprint(L"%-20s%i", WidenString(itr->script).c_str(), itr->persistent);
		}
		return e_command_result::kProcessed;
	}
}}}