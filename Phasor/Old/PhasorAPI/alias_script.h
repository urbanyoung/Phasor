/*! \file alias_script.h
 *	\brief Alias querying functions that correctly return the result.
 *	
 *	All alias functions pass a table to your specified callback function. Each
 *	key in the table is the player's hash, and the value is a table consisting
 *	of the player names.
 *	
 *	\b Important: You should use these functions and not \c svcmd if you
 *	want alias results.
 *	
 *	\addtogroup PhasorAPI
 *	@{
 */
#pragma once

#include "PhasorAPI.h"

/*! \brief Executes sv_alias_search and provides a callback for returning results.
 * 
 *	\param query The player to search for, see sv_alias_search for details.
 *	\param callback The name of a function to call when results are ready.
 *	\return Boolean indicating if the query can be executed.
 *	
 *	Example usage:
 *	\code
 *		function alias_hash_callback(results)
 *			for hash, names in pairs(results) do
 *				for k, name in pairs(names) do
 *					hprintf(hash .. " " .. name)
 *				end
 *			end
 *		end
 *		
 *		-- search for all names starting with New
 *		alias_search("New%", "alias_hash_callback")
 *	\endcode
*/	
void l_alias_search(PHASOR_API_ARGS);

/*! \brief Executes sv_alias_hash and provides a callback for returning results.
 *
 *	\param hash The hash to search for.
 *	\param callback The name of a function to call when results are ready.
 *	\return Boolean indicating if the query can be executed.
 *	
 *	 Example usage:
 *	\code
 *		function alias_callback(results)
 *			for hash, names in pairs(results) do
 *				for k, name in pairs(names) do
 *					hprintf(hash .. " " .. name)
 *				end
 *			end
 *		end
 *		
 *		-- search for all names matching the hash
 *		alias_hash("put some hash here", "alias_callback")
 *	\endcode
 */	
void l_alias_hash(PHASOR_API_ARGS);

//! }@