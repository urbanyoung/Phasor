/*! \file ScriptingChanges.h
	\brief Describes some of the important changes to the scripting system.
	
	Note: When I say "raise an error" it means that an error will be raised, and this
	error will be treated in the same way as other Lua errors. The calling function will get
	blacklisted.
	
	
	Deprecated (removed) functions:
		- \c gettoken, \c gettokencount, \c getcmdtoken and \c getcmdtokencount have been removed.
			- Instead you should use \c tokenizestring and \c tokenizecmdstring respectively.
		
	Changed functions:
		- \c hprintf no longer sends console text to the player executing rcon.
			- You should use \c respond to reply to rcon events.
			- hprintf(player, msg) is overloaded to \c sendconsoletext and therefore still works.
		- <b>Memory related functions are stricter</b>. If you pass an invalid memory address,
		  or try to write invalid data they will raise an error.
			- You should read their new documentation for more information.
		- <b>Player related functions are stricter</b>. All functions, apart from \c getplayer will
		  raise an error if you pass an invalid player.
			- You should always check if a player exists with \c getplayer, which returns \c nil if they don't
		- <b>Object related functions are stricter</b>. All functions, apart from \c getobject will
		  raise an error if you pass an invalid object.
			- This shouldn't cause any issues unless you're reading object ids from the wrong place.
*/
