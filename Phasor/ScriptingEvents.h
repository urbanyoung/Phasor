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
 *		- OnClientUpdate only receives the player's memory id, not their object id.
 *		- OnPlayerJoin only receives the player's memory id, not their team too.
 * 
 */

#pragma once

#include "Scripting.h"
#include "Common/types.h"
#include <string>

namespace halo
{
	struct ident;
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
		bool OnTeamChange(const halo::s_player& player, bool relevant, DWORD old_team,
			DWORD new_team);

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

		/*! \brief Called when a new game is starting.
		 *	
		 *	\param map The map the game is running.
		 *	
		 *	Definition:
		 *	\code
		 *		function OnNewGame(map)
		 *	\endcode
		 */
		void OnNewGame(const std::string& map);

		/*! \brief Called when a game is ending.
		 *
		 *	\param stage The stage of the game that has ended.
		 *	
		 *  \remark \c stage can be either:
		 *			- 1	  The game has just ended, and the ingame score card is shown.
		 *			- 2   The postgame scorecard is shown.
		 *			- 3   Players can now leave.
		 *			
		 *	Definition:
		 *	\code
		 *		function OnGameEnd(stage)
		 *	\endcode
		 */
		void OnGameEnd(DWORD stage);
		  
		/*! \brief Called when a player is attempting to join.
		 *
		 *	\param hash The joining player's hash
		 *	\param ip The joining player's ip
		 *	\return Boolean indicating whether or not the player is allowed to join.
		 *	
		 *	Definition:
		 *	\code
		 *		function OnBanCheck(hash, ip)
		 *	\endcode
		 */
		bool OnBanCheck(const std::string& hash, const std::string& ip);

		/*! \brief Called when a client sends its update packet
		 *
		 *	\param player The player's memory id.
		 *	
		 *	\remark This function is called 30 times a second for every
		 *	player in the server. Do not process this event unless you 
		 *	absolutely have to, and when processing it make your code as
		 *	efficient as possible.
		 *	
		 *	Definition:
		 *	\code
		 *		function OnClientUpdate(player)
		 *	\endcode
		 */
		void OnClientUpdate(const halo::s_player& player);

		/*! \brief Called when a player successfully joins the game.
		 *
		 *	\param player The joining player's memory id.
		 *	
		 *	Definition:
		 *	\code
		 *		function OnPlayerJoin(player)
		 *	\endcode
		 */
		void OnPlayerJoin(const halo::s_player& player);

		/*! \brief Called when a player quits.
		 * 
		 *	\param player The player who left.
		 *	
		 *	Definition:
		 *	\code
		 *		function OnPlayerLeave(player)
		 *	\endcode
		 */
		void OnPlayerLeave(const halo::s_player& player);

		/*! \brief Called when a player needs to be assigned a team.
		 *
		 *	\param team The team the player is about to be put on.
		 *	\return The team you want the player on, or \c nil if you don't care.
		 *	
		 *	Definition:
		 *	\code
		 *		function OnTeamDecsion(team)
		 *	\endcode
		 */
		bool OnTeamDecision(DWORD in_team, DWORD& out_team);

		/*! \brief Called when a player has spawned.
		 * 
		 *	\param player The player who is spawning.
		 *	\param objid The player's new object id.
		 *	
		 *	Definition:
		 *	\code
		 *		function OnPlayerSpawn(player, m_objectId)
		 *	\endcode
		 */
		void OnPlayerSpawn(const halo::s_player& player, halo::ident m_objectId);

		/*! \brief Called when the server has been notified on the player's spawn.
		 * 
		 *	\param player The player who is spawning.
		 *	\param objid The player's new object id.
		 *	
		 *	Definition:
		 *	\code
		 *		function OnPlayerSpawnEnd(player, m_objectId)
		 *	\endcode
		 */
		void OnPlayerSpawnEnd(const halo::s_player& player, halo::ident m_objectId);
}}

