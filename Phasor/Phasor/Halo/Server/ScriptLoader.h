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
	 *	Example usage: sv_rscript_reload
	 */
	e_command_result sv_script_reload(void*, 
		commands::CArgParser& args, COutStream& out);

	/*! \brief Loads the specified script for the duration of the current map.
	 *
	 *	script The script to load.
	 *	
	 *	Example usage: sv_script_load somescript
	 */
	e_command_result sv_script_load(void*, 
		commands::CArgParser& args, COutStream& out);
}}}