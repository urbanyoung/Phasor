/*! \file misc_halo.h
 *	\brief Miscellaneous halo related functions.
 *
 *	\addtogroup PhasorAPI
 *	@{
 */
#pragma once

#include "../phasor-lua.hpp"

/*! \brief Changes the specified player's team.
 *	\param player The player's memory id.
 *	\param forcekill Boolean indicating whether or not to kill the player.
 *	\param [team] The player's new team (optional)
 *
 *	Example usage:
 *	\code
 *		changeteam(player, true)
 *	\endcode
 *
 *	\remark
 *	If you don't specify the player's new team, their new team is the logical NOT
 *	of their current team. ie red -> blue, blue -> red.
 */
int l_changeteam(lua_State* L);

/*! \brief Kill the specified player.
 *	\param player The player's memory id.
 *
 *	Example usage:
 *	\code
 *		kill(player)
 *	\endcode
 */
int l_kill(lua_State* L);

/*! \brief Gives the specified player active camo for a specified duration
 *	\param player The player's memory id
 *	\param duration Duration of the camo, in seconds.
 *
 *	Example usage:
 *	\code
 *		applycamo(player, 30) -- invis for 30 seconds
 *	\endcode
 */
int l_applycamo(lua_State* L);

/*! \brief Executes a server command.
 *	\param cmd The command to execute
 *	\param [result] Boolean indicating whether or not a result is wanted (see remarks)
 *
 *	Example usage:
 *	\code
 *		svcmd("sv_ban " .. resolveplayer(player))
 *		local output = svcmd("sv_mapcycle", true)
 *		for k,v in ipairs(output) do
 *			hprintf("result: " .. v)
 *		end
 *	\endcode
 *
 *	\remark
 *	Be careful when using player ids! Memory ids should not be used when referring
 *	to a player. \c resolveplayer should be used to convert a memory id into
 *	an rcon id.
 *
 *	\remark
 *	If \c result is true then all text that would be sent to the executing
 *	player will be logged. Once the command has completed it will be returned
 *	to you as a table.
 */
int l_svcmd(lua_State* L);

/*! \brief Executes a server command as the specified player.
 *	\param cmd The command to execute
 *	\param player \b memory id of the player to execute as.
 *	\param [result] Boolean indicating whether or not a result is wanted (see remarks)
 *
 *	Example usage:
 *	\code
 *		svcmdplayer("sv_ban " .. resolveplayer(player), 0)
 *		local output = svcmd("sv_mapcycle", 0, true)
 *		for k,v in ipairs(output) do
 *			hprintf("result: " .. v)
 *		end
 *	\endcode
 *
 *	\remark
 *	Be careful when using player ids! Memory ids should not be used when referring
 *	to a player. \c resolveplayer should be used to convert a memory id into
 *	an rcon id.
 *
 *	\remark
 *	If \c result is true then all text that would be sent to the executing
 *	player will be logged. Once the command has completed it will be returned
 *	to you as a table.
 */
int l_svcmdplayer(lua_State* L);

/*! \brief Forcibly sync the specified weapon's ammo counts.
 *	\param weaponId The memory id of the weapon to sync.
 *
 *	Example usage:
 *	\code
 *		updateammo(m_weaponId) -- update the weapon's ammo
 *	\endcode
 */
int l_updateammo(lua_State* L);

/*! \brief Set a weapon's ammo and forcibly sync
 *	\param weaponId The memory id of the weapon to sync.
 *	\param clip_ammo How many rounds the weapon's clip should have.
 *	\param pack_ammo How many rounds the weapon has unloaded (not in clip)
 *
 *	Example usage:
 *	\code
 *		setammo(m_weaponId, 50, 200) -- 50 clip, 200 pack
 *	\endcode
 */
int l_setammo(lua_State* L);

/*! \brief Set the specified player's speed
 *	\param player The player's memory id
 *	\param speed The player's new speed (1.0 is default)
 *
 *	Example usage:
 *	\code
 *		setspeed(player, 5) -- 5x speed
 *	\endcode
 */
int l_setspeed(lua_State* L);

/*! \brief Gets halo's data directory (where banned.txt is stored)
 *	\return The profile directory
 *
 *	Example usage:
 *	\code
 *		local profilepath = getprofilepath()
 *	\endcode
 *	*/
int l_getprofilepath(lua_State* L);

/*! \brief Gets the server's current name
 *	\return The server's name
 *
 *	Exampe usage:
 *	\code
 *		local servername = getservername()
 *		hprintf(servername)
 *	\endcode
 */
int l_getservername(lua_State* L);

//! }@