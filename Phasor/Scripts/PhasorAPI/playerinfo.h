/*! \file playerinfo.h
 * \brief Player information querying functions for scripts.
 *
 * \b Important: All functions, other than \c getplayer, will raise a Lua error if
 * you send an invalid player. \c getplayer will return nil and can be used
 * to determine if a particular player is valid.
 *
 *	\addtogroup PhasorAPI
 *	@{
 */

#pragma once

#include "../phasor-lua.hpp"

/*! \brief Resolve a player's memory id to their rcon id.
 *
 * \param player The memory id of the player to resolve
 * \return The player's rcon id
 *
 * Example usage:
 * \code
 *		local player_rconid = resolveplayer(0)
 * \endcode
 */
int l_resolveplayer(lua_State* L);

/*! \brief Resolve a player's rcon id to their memory id.
 *
 * \param player The rcon id of the player to resolve.
 * \return The player's memory id
 *
 * Example usage:
 * \code
 *		local player_memid = rresolveplayer(0)
 * \endcode
 */
int l_rresolveplayer(lua_State* L);

/*! \brief Get the specified player's memory data.
 *
 * \param player The player's memory id.
 * \return The address of the player's memory info
 *
 * Example usage:
 * \code
 *		local m_player = getplayer(0)
 * \endcode
 */
int l_getplayer(lua_State* L);

/*! \brief Get the specified player's ip address.
 *
 * \param player The player's memory id.
 * \return The player's ip address
 *
 * Example usage:
 * \code
 *		local ip = getip(3)
 * \endcode
 */
int l_getip(lua_State* L);

/*! \brief Get the specified player's network port
 *
 * \param player The player's memory id.
 * \return The player's network port
 *
 * Example usage:
 * \code
 *		local ip = getip(0)
 *		local port = getport(0)
 *		hprintf("Player with ip:port " .. ip .. ":" .. port)
 * \endcode
 * Example output:
 * \verbatim
 Player with ip:port 127.0.0.1:2302
 \endverbatim
 */
int l_getport(lua_State* L);

/*! \brief Get the specified player's team
 *
 * \param player The player's memory id.
 * \return The player's current team.
 *
 * Example usage:
 * \code
 *		local team = getteam(5)
 * \endcode
 */
int l_getteam(lua_State* L);

/*! \brief Get the specified player's name
 *
 * \param player The player's memory id.
 *
 * Example usage:
 * \code
 *		local name = getname(0)
 * \endcode
 */
int l_getname(lua_State* L);

/*! \brief Get the specified player's hash
 *
 * \param player The player's memory id.
 * \return The player's hash.
 *
 * Example usage:
 * \code
 *		local hash = gethash(player)
 * \endcode
 */
int l_gethash(lua_State* L);

/*! \brief Get the number of players on the specified team
 *
 * \param team The team to check (0 - red, 1 - blue)
 * \return The number of players on the specified team
 *
 * Example usage:
 * \code
 *		local redsize = getteamsize(0) -- get size of red team
 *		local bluesize = getteamsize(1) -- get size of blue team
 * \endcode
 */
int l_getteamsize(lua_State* L);

/*! \brief Get the specified player's object address.
*
* \param player The player's memory id.
* \return The player's object address or nil if they are dead.
*
* \remark
* Equivalent to: getobject(getplayerobjectid(0)
*
* Example usage:
* \code
*		local player_obj_id = getplayerobject(0) -- get player 0's object id
* \endcode
*
* \remark
* If the player is currently dead, nil is returned.
*/
int l_getplayerobject(lua_State* L);

/*! \brief Get the specified player's object id.
 *
 * \param player The player's memory id.
 * \return The player's object id or nil if they are dead.
 *
 * Example usage:
 * \code
 *		local player_obj_id = getplayerobjectid(0) -- get player 0's object id
 * \endcode
 *
 * \remark
 * If the player is currently dead, nil is returned.
 */
int l_getplayerobjectid(lua_State* L);

/*! \brief Checks if the specified player is an admin.
 *
 * \param player The player's memory id.
 * \return boolean indicating whether or not the player is an admin.
 *
 * Example usage:
 * \code
 *		local is_player_admin = isadmin(0)
 * \endcode
 */
int l_isadmin(lua_State* L);

/*! \brief Returns the admins level, or \c nil if not an admin.
 *
 * \param player The player's memory id.
 * \return integer representing the player's level, or \c nil if not admin.
 *
 * Example usage:
 * \code
 *		local admin_lvl = getadminlvl(0)
 *		if (admin_lvl ~= nil) then
 *			hprintf("{0} is admin at level " .. admin_lvl)
 *		end
 *	\endcode
 */
int l_getadminlvl(lua_State* L);

/*! \brief Sets the specified player as an admin for the duration of the
 * current game, or until the player leaves.
 *
 * \param player The player's memory id.
 *
 * Example usage:
 * \code
 *		setadmin(0) -- player 0 is now an admin
 *		if (isadmin(0) == true) then
 *			-- this will always be true
 *		end
 * \endcode
 */
int l_setadmin(lua_State* L);

//! }@