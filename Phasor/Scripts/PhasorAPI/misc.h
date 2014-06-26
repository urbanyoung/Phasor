/*! \file misc.h
 * \brief Miscellaneous scripting functions that aren't Halo related.
 *
 *	\addtogroup PhasorAPI
 *	@{
 */
#pragma once

#include "../phasor-lua.hpp"

/*! \brief Get the current cpu ticks.
 *
 * \return The current cpu ticks
 *
 * Example usage:
 * \code
 *		local ticks = gettick()
 *	\endcode
 */
int l_getticks(lua_State* L);

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
int l_getrandomnumber(lua_State* L);

//! }@