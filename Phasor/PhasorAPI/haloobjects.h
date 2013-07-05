/*! \file objects.h
 *	\brief Object related functions for scripts.
 *	
 *	\b Important: If you pass an invalid object id to any of these functions,
 *	except for \c getobject or \c gettagid a Lua error is raised.
 *	
 *	##Changes:
 *		- \c createobject now takes the tag id, not tag type and name. You can
 *			use \c gettagid to get a tag's id.
 *			
 *	\addtogroup PhasorAPI
 *	@{
 */
#pragma once
#include "PhasorAPI.h"

/*! \brief Get the memory address for the specified object.
 *	\param object_id The object's id
 *	\return The objects memory id, or \c nil if the specified id is invalid.
 *	
 *	Example usage:
 *	\code
 *		local m_object = getobject(object_id)
 *		if (m_object ~= nil) then
 *			-- object is valid
 *		end
 *	\endcode
 */
void l_getobject(PHASOR_API_ARGS);

/*! \brief Get the specified object's memory address.
 *	\param object_id The object's id
 *	\return x,y,z The object's x, y and z coordinates.
 *
 *	Example usage:
 *	\code
 *		local x,y,z = getobjectcoords(object_id)
 *	\endcode
 */
void l_getobjectcoords(PHASOR_API_ARGS);

/*! \brief Find which player the specified memory address belongs to.
 *	\param m_object The object's memory address
 *	\return memory_id The player's memory id, or \c nil if not found.
 *	
 *	Example usage:
 *	\code
 *		local player = objecttoplayer(m_object)
 *		if (player ~= nil) then
 *			hprintf("object belongs to " .. player)
 *		end
 *	\endcode
 *
 * \remark This only works for player objects, not objects created by the player,
 * ie bullets.
 */
void l_objecttoplayer(PHASOR_API_ARGS);

/*! \brief Creates an ingame object.
 *
 *	\param tag_id The id of the tag describing the object to create.
 *	\param parentId The object id to assign as the new one's parent.
 *	\param respawnTime Number of seconds to wait before respawning the object.
 *	\param do_respawn Boolean indicating whether or not the object should respawn.
 *	\param x The object's x coordinate.
 *	\param y The object's y coordinate.
 *	\param z The object's z coordinate.
 *	\return The created object's id or \c nil on failure.
 *	
 *	\remark
 *		- If you don't what a parent, set \c parentId to 0. You won't care about this most of the time.
 *		- \c do_respawn indicates whether or not the object should respawn (go to spawn
 *		coordinates) or whether it should be destroyed.
 *		- \c do_respawn is ignored when creating equipment or weapons. They are
 *		always destroyed.
 *		
 *	Example usage:
 *	\code
 *		function OnNewGame(map)
 *			global assault_rifle_id = LookupTag("weap", "weapons\\assault rifle\\assault rifle")
 *		end
 *		
 *		... (somewhere else in the code) ...
 *		
 *		objid = createobject(assault_rifle_id, 0, 10, false, 1,2,3)
 *		if (objid == nil) then hprintf("failed to make object") end
 *	\endcode
 */
void l_createobject(PHASOR_API_ARGS);

/*! \brief Destroys the specified objects.
 *
 *	\param objid The id of the object to destroy.
 *	
 *	\remark if \c objid is not a valid object id, an error is raised.
 *	
 *	Example usage:
 *	\code
 *		destroyobject(objid)
 *	\endcode
*/
void l_destroyobject(PHASOR_API_ARGS);

/*! \brief Gives and equips the specified weapon to the specified player.
 *
 *	\param player The player to assign the weapon to.
 *	\param weapId The id of the weapon to assign.
 *	\return Boolean indicating whether or not the assignment was successful.
 *	
 *	Example usage:
 *	\code
 *		assignweapon(player, weaponid)
 *	\endcode
 */
void l_assignweapon(PHASOR_API_ARGS);

/*! \brief Forces the specified player into the specified vehicle.
 *
 *	\param player The player to make enter the vehicle.
 *	\param vehicleid The object id of the vehicle the player is to enter.
 *	\param seat The seat the player should enter.
 *	
 *	\remark
 *	Possible seat values:
 *		- 0 Driver
 *		- 1 Passenger
 *		- 2 Gunner
 *	
 *	Example usage:
 *	\code
 *		entervehicle(player, vehicleid, 0)
 *	\endcode
 */
void l_entervehicle(PHASOR_API_ARGS);

/*! \brief Checks whether the specified player is in a vehicle.
 *
 *	\param player The player to check.
 *	\return Boolean indicating whether or not the player is in a vehicle.
 *	
 *	Example usage:
 *	\code
 *		local in_vehicle = isinvehicle(player)
 *		if (in_vehicle) then hprintf("in vehicle")
 *		else hprintf("not in vehicle")
 *		end
 *	\endcode
 */
void l_isinvehicle(PHASOR_API_ARGS);

/*! \brief Forces the specified player to leave their current vehicle.
 *
 *	\param player The player to eject from their vehicle.
 *	
 *	Example usage:
 *	\code
 *		exitvehicle(player)
 *	\endcode
 */
void l_exitvehicle(PHASOR_API_ARGS);

/*! \brief Moves the specified object to the specified coordinates.
 * 
 *  \param objid The id of the object to move.
 *  \param x The x coordinate to move the object to.
 *  \param y The y coordinate to move the object to.
 *  \param z The z coordinate to move the object to.
 *  
 *  Example usage:
 *  \code
 *		local playerobjid = getplayerobjectid(player)
 *		if (playerobjid ~= nil) then
 *			movobjectcoords(playerobjid, 5, 10, 15)
 *		end
 * \endcode
 */
void l_movobjectcoords(PHASOR_API_ARGS);

/*! \brief Gets the map id of the specified tag.
 * 
 *	\param tagType The type of the tag
 *	\param tagName The name of the tag
 *	\return The tag's map id or \c nil if not found.
 *	
 *	\remark 
 *	You should lookup all tags (via this function) in OnNewGame
 *	and store the results globally. Looking up tags based on their name
 *	is slow.
 *	
 *	Example usage:
 *	\code
 *		function OnNewGame(map)
 *			global ar_tag_id = gettagid("weap", "weapons\\assault rifle\\assault rifle")
 *		end
 * \endcode
 */
void l_gettagid(PHASOR_API_ARGS);

/*! \brief Gets the tag type and tag name of the specified tag (map) id.
 *
 *	\param tagid The tag id to lookup.
 *	\return \c tag_name, \c tag_type
 *	
 *	\remark
 *	You shouldn't compare tag names, look up the ones you want in \c OnNewGame
 *	
 *	Example usage:
 *	\code
 *		local tag_name, tag_type = gettaginfo(some_tag_id)
 *	\endcode
 */
void l_gettaginfo(PHASOR_API_ARGS);

/*! \brief Gets the specified tag's memory address.
 *
 *	\param tagid The id of the tag whose address is to be found.
 *	\return The tag's memory address
 *	
 *	\remark
 *	This is similar to \c lookuptag which has been deprecated.
 *	
 *	Example usage:
 *	\code
 *		local tag_data = gettagaddress(ar_tag_id)
 *	\endcode
 */
void l_gettagaddress(PHASOR_API_ARGS);

/*! \brief Applies damage to the specified object.
 *
 * \param receiver object id of the object which will receive the damage.
 * \param multiplier real number indicating how much damage to apply.
 * \param [causer] object id of the object to attribute the damage to.
 * \param [flags] damage flags to use (see odl_flags)
 *
 * Example usage:
 * \code
 *		-- damage player 0, give credit to player 1 and ignore shields
 *		applydmg(getplayerobjectid(0), 2.0, getplayerobjectid(1), 4)
 * \endcode
 *
 */
void l_applydmg(PHASOR_API_ARGS);

/*! \brief Applies a damage tag to the specified object.
 *
 * \param receiver object id of the object which will receive the damage.
 * \param dmg_tag tag id of the damage tag to apply.
 * \param [multiplier] real number indicating the damage multiplier (default 1.0)
 * \param [causer] object id of the object to attribute the damage to.
 * \param [flags] damage flags to use (see odl_flags)
 * 
 * \remark
 * You cannot set the hit location (might add this later). I think it 
 * will default to a body shot.
 *
 * Example usage:
 * \code
 *		-- apply a pistol shot to player 0
 *		local pistol_dmg_id = gettagid("jpt!", "weapons\\pistol\\bullet")
 *		applydmgtag(getplayerobjectid(0), pistol_dmg_id)
 * \endcode
 *
 */
void l_applydmgtag(PHASOR_API_ARGS);

//! }@