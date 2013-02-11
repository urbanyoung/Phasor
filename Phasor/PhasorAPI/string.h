/*! \file string.h
 * \brief String manipulation functions provided by Phasor
 * 
 * Provides the string manipulation functions to scripts.
 * 
 * Note: These implementations have changed since the previous Phasor. Both
 * tokenizestring and tokenizecmdstring return an array of strings and as 
 * such gettoken, getcmdtoken, gettokencount, getcmdtokencount have
 * been removed.
 */

#pragma once

#include "PhasorAPI.h"

/*! \brief Tokenizes (splits) an input string at specified delimiters.
 *
 * \param str The string to tokenize
 * \param delim A string of characters to split at.
 * \return A table consisting of the split strings.
 */
void l_tokenizestring(PHASOR_API_ARGS);

/*! \brief Tokenizes (splits) an input string into distinct words, which are
 * either separated by a space or enclosed in " ".
 *
 * \param str The string to tokenize
 * \return A table consisting of the split strings.
 */
void l_tokenizecmdstring(PHASOR_API_ARGS);
