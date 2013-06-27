#pragma once

#include "../../../Common/Types.h"

#include <string>

enum e_command_result;

namespace commands
{
	class CArgParser;
}
class COutStream;

namespace halo { namespace server { namespace scriptloader
{	
	void LoadScripts();
	bool IsValidScript(const std::string& script);

	/*! \brief Reloads the specified script, or if none specified all scripts.
	 * 
	 *	[script] The script to reload (optional)
	 *
	 *	Example usage: sv_script_reload
	 */
	e_command_result sv_script_reload(void*, 
		commands::CArgParser& args, COutStream& out);

	/*! \brief Loads the specified script for the duration of the current map.
	 *
	 *	script The script to load.
	 *	persistent Whether or not the script is persistent (shouldn't unload between games)
	 *	
	 *	Example usage: sv_script_load somescript true
	 */
	e_command_result sv_script_load(void*, 
		commands::CArgParser& args, COutStream& out);

	/*! \brief Unloads the specified script
	 * 
	 *	script The script to unload
	 *
	 *	Example usage: sv_script_unload testscript
	 */
	e_command_result sv_script_unload(void*, 
		commands::CArgParser& args, COutStream& out);

	/*! \brief Lists all currently loaded scripts. 
	 * 
	 *  Example usage: sv_script_list
	 */
	e_command_result sv_script_list(void*, 
		commands::CArgParser& args, COutStream& out);
}}}