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
#include "../phasor-lua.hpp"

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
int l_getobject(lua_State* L);

/*! \brief Get the specified object's location in the map.
*	\param object_id The object's id
*	\return x,y,z The object's x, y and z coordinates.
*
*	Example usage:
*	\code
*		local x,y,z = getobjectcoords(object_id)
*	\endcode
*/
int l_getobjectcoords(lua_State* L);

/*! \brief Find which player the specified memory address belongs to.
*	\param m_object The object's memory address
*	\return memory_id The player's memory id, or \c nil if not found.
*
*	Example usage:
*	\code
*		local player = objectaddrtoplayer(m_object)
*		if (player ~= nil) then
*			hprintf("object belongs to " .. player)
*		end
*	\endcode
*
* \remark This only works for player objects, not objects created by the player,
* ie bullets.
*/
int l_objectaddrtoplayer(lua_State* L);

/*! \brief Find which player the specified object id belongs to.
*	\param m_objectId The object's id
*	\return memory_id The player's memory id, or \c nil if not found.
*
*	Example usage:
*	\code
*		local player = objectidtoplayer(objid)
*		if (player ~= nil) then
*			hprintf("object belongs to " .. player)
*		end
*	\endcode
*
* \remark This only works for player objects, not objects created by the player,
* ie bullets.
*/
int l_objectidtoplayer(lua_State* L);

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
*			assault_rifle_id = LookupTag("weap", "weapons\\assault rifle\\assault rifle")
*		end
*
*		... (somewhere else in the code) ...
*
*		objid = createobject(assault_rifle_id, 0, 10, false, 1,2,3)
*		if (objid == nil) then hprintf("failed to make object") end
*	\endcode
*/
int l_createobject(lua_State* L);

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
int l_destroyobject(lua_State* L);

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
int l_assignweapon(lua_State* L);

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
int l_entervehicle(lua_State* L);

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
int l_isinvehicle(lua_State* L);

/*! \brief Forces the specified player to leave their current vehicle.
*
*	\param player The player to eject from their vehicle.
*
*	Example usage:
*	\code
*		exitvehicle(player)
*	\endcode
*/
int l_exitvehicle(lua_State* L);

/*! \brief Moves the specified object to the specified coordinates.
*
*  \param objid The id of the object to move.
*  \param x The x coordinate to move the object to.
*  \param y The y coordinate to move the object to.
*  \param z The z coordinate to move the object to.
*
*  \remark
*  This function will also reset the object's flags to indicate
*  that it is not stationary. This causes gravity to work as expected.
*  If this is no the behaviour you want, then do \c writebit(m_obj + 0x10, 5, 1)
*
*  Example usage:
*  \code
*		local playerobjid = getplayerobjectid(player)
*		if (playerobjid ~= nil) then
*			movobjectcoords(playerobjid, 5, 10, 15)
*		end
* \endcode
*/
int l_movobjectcoords(lua_State* L);

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
int l_gettagid(lua_State* L);

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
int l_gettaginfo(lua_State* L);

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
int l_gettagaddress(lua_State* L);

/*! \brief Applies damage to the specified object.
*
* \param receiver object id of the object which will receive the damage.
* \param multiplier real number indicating how much damage to apply.
* \param [causer] object id of the object to attribute the damage to.
* \param [flags] damage flags to use (see odl_flags)
*
* \remarks
* flags is a bitfield, the same as described in odl_flags. However, applydmg
* requires the actual flags value - you don't set each bit, like in odl_flags.
* So, to calculate the actual value use the following eqn:
* local flags = 2^0 + 2^2 + 2^5
* That will activate bits 0, 2 and 5, which indicates vehicle hits, instant
* kill and ignore shields.
*
* Example usage:
* \code
*		-- damage player 0, give credit to player 1 and ignore shields
*		applydmg(getplayerobjectid(0), 2.0, getplayerobjectid(1), 4)
* \endcode
*
*/
int l_applydmg(lua_State* L);

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
int l_applydmgtag(lua_State* L);

/*! \brief Checks if the specified vector intersects anything.
*
*	\param dist the direction vector is multiplied by this value, the intersection
*	test stops after the result of this product.
*	\param x x coordinate for start of ray.
*	\param y y coordinate for start of ray.
*	\param z z coordinate for start of ray.
*	\param vx x direction for ray.
*	\param vy y direction for ray.
*	\param vz z direction for ray.
*	\param [objid] id of object to ignore during collision tests
*	\return hit,x,y,z,obj
*
*	\remark
*	If you are firing a ray from a player's perspective, you should supply
*	the optional argument \c objid as the player's object id. This will ensure
*	they are ignored from collision tests, and the ray will not intersect with
*	them (otherwise it will be trapped in their head).
*
*	\remark
*	\c hit is set if the ray intersects with something (wall, obj etc)\n
*	\c x,\cy,\cz is the hit location.\n
*	\c obj is the object id of the first object hit, or \c nil if none.
*
*	Example usage:
*	\code
local player_objid = getplayerobjectid(player)
if (player_objid ~= nil) then
local m_object = getobject(player_objid)
local vx = readfloat(m_object + 0x230)
local vy = readfloat(m_object + 0x234)
local vz = readfloat(m_object + 0x238)
local px, py, pz = getobjectcoords(player_objid)

-- We want to fire the ray from the player's head
-- So we need to find the standing/crouch height for the map
-- You should move this little bit of code into OnNewGame and
-- save the values.
local bipd_id = readdword(m_object)
local bipd_tag = gettagaddress(bipd_id)
local bipd_data = readdword(bipd_tag + 0x14)
local standing_height = readfloat(bipd_data + 0x400)
local crouch_height = readfloat(bipd_data + 0x404)

local crouch_state = readfloat(m_object + 0x50c)
if (crouch_state == 0) then pz = pz + standing_height
else pz = pz + (crouch_height * crouch_state) end

local hit,x,y,z,objid = halointersect(1000, px, py, pz, vx, vy, vz, player_objid)
if (hit == true) then
hprintf(string.format("The hit location is (%.2f, %.2f, %.2f)", x,y,z))
if (objid ~= nil) then
hprintf(string.format("The player is looking at object %08X", objid))
end
else
hprintf("no hit")
end
end
*	\endcode
*/
int l_halointersect(lua_State* L);

//! }@