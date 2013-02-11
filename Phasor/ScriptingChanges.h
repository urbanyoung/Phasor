/*! \file ScriptingChanges.h
	\brief Describes some of the important changes to the scripting system.
	
	Deprecated (removed) functions:
		- \b gettoken, \b gettokencount, \b getcmdtoken and \b getcmdtokencount have been removed.
			- Instead you should use \b tokenizestring and \b tokenizecmdstring respectively.
		
	Changed functions:
		- \b hprintf no longer sends console text to the player executing rcon.
			- You should use \b respond to reply to rcon events.
			- hprintf(player, msg) is overloaded to \b sendconsoletext and therefore still works.
		- <b>Memory related functions are now stricter</b>. If you pass an invalid memory address,
		  or try to write invalid data they will raise an error.
			- You should read their new documentation for more information.
		- <b>Player related functions are stricter</b>. All functions, apart from \b getplayer will
		  raise an error if you pass an invalid player.
			- You should always check if a player exists with \b getplayer, which returns \b nil if they don't
*/
