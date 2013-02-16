/*! \file objects.h
 *	\brief Object related functions for scripts.
 *	
 *	\b Important: If you pass an invalid object id to any of these functions,
 *	except for \c getobject a Lua error is raised.
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
 *	\end
 *
 * \remark This only works for player objects, not objects created by the player,
 * ie bullets.
 */
void l_objecttoplayer(PHASOR_API_ARGS);

