/*! \file damagelookup.h
 * \brief Callback functions for use in OnDamageLookup.
 *
 * These functions can be used to change the behaviour of OnDamageLookup. They
 * should be used if you want to change the damage tag, causer or receiver
 * etc. 
 * 
 * \remark
 * Only call these functions from within OnDamageLookup.
 * 
 *	\addtogroup PhasorAPI
 *	@{
*/

#include "PhasorAPI.h"
#include "../../Phasor/Halo/Halo.h"

namespace halo { struct s_damage_info; struct damage_script_options; }
namespace odl {
	
	void setData(halo::damage_script_options* opts_, 
		halo::s_damage_info* dmg, const halo::ident& receiver);
	void reset();
}

/*! \brief Sets the casuer of the damage to the specified object.
 *
 *	\param id object id to attribute damage to. Can be 0xFFFFFFFF to specify
 *	that no object is causing the damage.
 * 
 * 	Example usage:
 *  \code
 *		odl_causer(getplayerobjectid(0))
 *  \endcode
 */
void l_odl_causer(PHASOR_API_ARGS);

/*! \brief Sets the receiver of the damage to the specified object.
 *
 *	\param id object id to attribute damage to.
 * 
 * 	Example usage:
 *  \code
 *		odl_receiver(getplayerobjectid(1))
 *  \endcode
 */
void l_odl_receiver(PHASOR_API_ARGS);

/*! \brief Can be used to change the type of damage which is being applied.
 *
 *	\param id object id of the damage tag to apply.
 *	
 *	\remarks
 *	Will raise an error if \c id is not a damage (jpt!) tag.
 * 
 * 	Example usage:
 *  \code
 *		odl_tag(gettagid("jpt!", "globals\\falling"))
 *  \endcode
 */
void l_odl_tag(PHASOR_API_ARGS);

/*! \brief Can be used to change the amount of damage being done.
 *
 *	\param dmg A real number representing the damage multiplier.
 * 
 *  \remarks
 *  To block damage return false in OnDamageLookup, don't use a modifier
 *  of 0.
 * 
 * 	Example usage:
 *  \code
 *		odl_multiplier(10.0)
 *  \endcode
 */
void l_odl_multiplier(PHASOR_API_ARGS);

/*! \brief Read/Write data to the flags used in damage lookup.
 * 
 * \param bit_offset The offset (starting at 0) to access.
 * \param [new_value] The new value for the specified bit.
 * \return The value of the bit, if new_value is not specified, or \c nil otherwise.
 *  
 *  \remark
 *	Here are some bit offsets I found:\n
 *		0 - Hits on player's vehicle don't do damage, only player hits do.\n
 *		2 - Instant kill.\n
 *		5 - Ignore shields.\n
 *		
 *		
 * Example usage:
 *  \code
 *		local instant_kill = odl_flags(2)
 *		-- or alternatively, to set it...
 *		odl_flags(2, 1)
 *  \endcode
 */
void l_odl_flags(PHASOR_API_ARGS);

//! }@