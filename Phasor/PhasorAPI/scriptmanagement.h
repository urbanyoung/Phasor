/*! \file scriptmanagement.h
 * \brief Functions scripts can use to manage their execution. More will
 * be added soon.
 * 
 *	\addtogroup PhasorAPI
 *	@{
 */

#pragma once

#include "PhasorAPI.h"

/*! \brief Raises an error to Phasor's scripting subsystem.
 *
 * \param desc A string describing the error you are raising.
 * 
 * \remark
 * This function doesn't return and the script function that calls it
 * will be considered in-error. It produces equivalent behaviour to 
 * any other scripting errors.
 * 
 * Example usage:
 * \code
 *		function OnScriptLoad(processid, game, persistent)
 *			if (persistent == true) then
 *				raiseerror("this script doesn't support persistance.")
 *			end
 *		end
 * \endcode
 */
void l_raiseerror(PHASOR_API_ARGS);