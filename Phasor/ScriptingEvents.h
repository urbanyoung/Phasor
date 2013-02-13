/*! \namespace scripting::events
 * \brief All script events are raised through functions here.
 *
 *	All scripting events receive at least one parameter, this parameter
 *	determines whether or not your return value will be considered.
 *	It is the last parameter on all functions and you don't need to explicitly
 *	add it to your parameter list unless you want to use it.
 *	
 *	### CHANGES
 *		- If you don't want your return value to be considered you \b must return \c nil
 *		- If you return a value and \c relevant == \c false it will be ignored.
 *		- The value indicating the server is executing the command is \c nil not \c -1
 *		
 * 
 */

#pragma once

#include "Scripting.h"
#include "Common/types.h"
#include <string>

namespace halo
{
	struct s_player;
}

namespace scripting {
	namespace events {

		/*! \brief Called when a player has changed team.
		 *
		 *	\param player The player who is changing team.
		 *	\param old_team The player's old team
		 *	\param new_team The player's new team
		 *	\param relevant Determines whether or not your return value matters.
		 *	\return boolean indicating whether or not the player should change team.
		 *	
		 *	\remark
		 *	It is important that if you process this function, you process \c relevant
		 *	correctly. 
		 *	
		 *	Definition:
		 *	\code
		 *		function OnTeamChange(player, old_team, new_team, relevant)
		 *	\endcode
		 */
		bool OnTeamChange(const halo::s_player& player, bool relevant, DWORD old_team);

		/*! \brief Called when a server command is being executed.
		 * 
		 * This function is only called when the player enters the correct 
		 * rcon password and when they are allowed to execute the desired command.
		 * 
		 * \param player The player executing the command
		 * \param command The command being executed.
		 * \return boolean indicating whether or not Phasor should process the command.
		 * 
		 * Definition:
		 * \code
		 *		function OnServerCommand(player, command)
		 *	\endcode
		 */
		bool OnServerCommand(const halo::s_player* player, const std::string& command);

}}

