/*! \file PhasorAPI.h
 *
 *	\addtogroup PhasorAPI
 *	\brief All functions Phasor provides are described here.
 * 
 *	Ignore the \c l_ infront of function names, that's just how I name them
 *	within my code. You don't use that when calling the functions.
 * 
 *	### CHANGES:
 *	Note: When I say "raise an error" it means that an error will be raised, and this
 *	error will be treated in the same way as other Lua errors. The calling function will get
 *	blacklisted.
 * 
 *	Deprecated (removed) functions:
 *	- \c gettoken, \c gettokencount, \c getcmdtoken and \c getcmdtokencount have been removed.
 *		- Instead you should use \c tokenizestring and \c tokenizecmdstring respectively.
 *	- \c lookuptag has been deprecated. Instead you should use \c gettagid or \c gettagaddress

 *	Changed functions:
 *	- \c hprintf no longer sends console text to the player executing rcon.
 *		- You should use \c respond to reply to rcon events.
 *		- hprintf(player, msg) is overloaded to \c sendconsoletext and therefore still works.
 *	- <b>Memory related functions are stricter</b>. If you pass an invalid memory address,
 *	or try to write invalid data they will raise an error.
 *		- You should read their new documentation for more information.
 *	- <b>Player related functions are stricter</b>. All functions, apart from \c getplayer will
 *	raise an error if you pass an invalid player.
 *		- You should always check if a player exists with \c getplayer, which returns \c nil if they don't
 *	- <b>Object related functions are stricter</b>. All functions, apart from \c getobject will
 *	raise an error if you pass an invalid object.
 *		- This shouldn't cause any issues unless you're reading object ids from the wrong place.
 *	- \c OnPlayerJoin no longer receives the player's team.
 *	
 *	### Further
 *	For more detailed information about the changes to specific groups of functions
 *	you should see Files/PhasorAPI and view the relevant file:
 *	
 *	alias_script.h haloobjects.h memory.h misc.h misc_halo.h output.h playerinfo.h
 *	scripttimers.h string.h
 *	
 */
#pragma once

#include "../Manager.h"

#define PHASOR_API_ARGS Manager::CallHandler& handler, \
	Common::Object::unique_deque& args, \
	Common::Object::unique_list& results

namespace PhasorAPI
{
	void Register(Manager::ScriptState& state);
}