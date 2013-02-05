/*! \file deprecated.h
 * \brief Implementations provided for backwards compatibility with old scripts.
 * 
 * All functions here are either deprecated (and to be removed) or have 
 * been subsequently updated and their usage changed.
 * 
 * All scripts with a version number prior to 01.00.10.059 use these functions.
 * 
 * The functions declared here are implemented in their respective files,
 * where the current versions reside, although in the deprecated namespace.
 */

#include "PhasorAPI.h"

namespace deprecated
{
	/*! \brief Outputs a string to the server console.
	 * \param str The string to print
	 * \param [player] The player to send message to, as console text.
	 * 
	 * Example usage:
	 * \code
	 *		hprintf("Hello player 1", 1)
	 * \endcode
	 */
	void l_hprintf(PHASOR_API_ARG);
}