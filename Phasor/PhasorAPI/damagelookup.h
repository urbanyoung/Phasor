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
#include "../Phasor/Halo/Halo.h"

namespace halo { struct s_damage_info; struct damage_script_options; }
namespace odl {
	
	void resetData(halo::damage_script_options* opts_, 
		halo::s_damage_info* dmg, const halo::ident& receiver);
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

/*! \brief Specifies that this damage, regardless of its intensity, should
 * instantly kill the target.
 * 
 * \param [boolean] whether or not instant kill should be enabled (true/false).
 * \return The previous value.
 * 
 * Example usage:
 *  \code
 *		local is_instant_kill = odl_flags_instantkill()
 *		-- or alternatively, to set it...
 *		odl_flags_instantkill(true)
 *  \endcode
 */
void l_odl_flags_instantkill(PHASOR_API_ARGS);

/*! \brief Specifies that this damage should be treated as self-inflicted
 * if the player dies.
 * 
 * \param [boolean] whether or not suicide should be enabled (true/false).
 * \return The previous value.
 * 
 * \remark
 * Halo does not apply suicides in this way, you should still check if
 * the causer is the same as the receiver. This is just an optional
 * flag which ignores the causer.
 * 
 * Example usage:
 *  \code
 *		local is_suicide = odl_flags_suicide()
 *		-- or alternatively, to set it...
 *		odl_flags_suicide(true)
 *  \endcode
 */
void l_odl_flags_suicide(PHASOR_API_ARGS);

//! }@