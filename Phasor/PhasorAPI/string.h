/*! \file string.h
 * \brief String manipulation functions provided by Phasor
 * 
 * These implementations have changed since the previous Phasor.\n
 * Both \c tokenizestring and \c tokenizecmdstring return an array of strings
 * and as such \c gettoken, \c getcmdtoken, \c gettokencount, \c getcmdtokencount have
 * been removed.
 * 
 *	\addtogroup PhasorAPI
 *	@{
 */

#pragma once

#include "PhasorAPI.h"

/*! \brief Tokenizes (splits) an input string at specified delimiters.
 *
 * \param str The string to tokenize
 * \param delim A string of characters to split at.
 * \return A table consisting of the split strings.
 * 
 * Example usage:
 * \code
 *		local tokens = tokenizestring("Split at each space,and,comma", " ,")
 *		-- print the tokens
 *		for k,v in ipairs(tokens) do
 *			hprintf("Token " .. k .. " is " .. v)
 *		end
 * \endcode
 * Output:
 * \verbatim
 Token 1 is Split
 Token 2 is at
 Token 3 is each
 Token 4 is space
 Token 5 is and
 Token 6 is comma
 \endverbatim
 */
void l_tokenizestring(PHASOR_API_ARGS);

/*! \brief Tokenizes (splits) an input string into distinct words, which are
 * either separated by a space or enclosed in " ".
 *
 * \param str The string to tokenize
 * \return A table consisting of the split strings.
 * 
 * Example usage:
 * \code
 *		local tokens = tokenizecmdstring("Split at space \"but not while in quotes\" ok?")
 *		-- print the tokens
 *		for k,v in ipairs(tokens) do
 *			hprintf("Token " .. k .. " is " .. v)
 *		end
 * \endcode
 * Output:
 * \verbatim
 Token 1 is Split
 Token 2 is at
 Token 3 is space
 Token 4 is but not while in quotes
 Token 5 is ok?
 \endverbatim
 */
void l_tokenizecmdstring(PHASOR_API_ARGS);

//! }@