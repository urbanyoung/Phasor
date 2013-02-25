/*! \file misc.h
 * \brief Miscellaneous scripting functions that aren't Halo related.
 * 
 *	\addtogroup PhasorAPI
 *	@{
 */
#pragma once

#include "PhasorAPI.h"

/*! \brief Get the current cpu ticks.
 *
 * \return The current cpu ticks
 * 
 * Example usage:
 * \code
 *		local ticks = gettick()
 *	\endcode
 */
void l_getticks(PHASOR_API_ARGS);

/*! \brief Generate a random number in the given range
 *
 * \param min The minimum value to generate (inclusive)
 * \param max The maximum value to generate (exclusive)
 * \return The number generated
 * 
 * Example usage:
 * \code
 *		local ticks = getrandomnumber(0, 10) -- between 0 and 10
 *	\endcode
 */
void l_getrandomnumber(PHASOR_API_ARGS);

//! }@