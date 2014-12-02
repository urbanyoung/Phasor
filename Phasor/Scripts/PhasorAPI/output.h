/*! \file output.h
 * \brief Text related output functions for scripts
 *
 * Provides the textual output related Phasor scripting functions.
 *
 * Notes:
 * All messages can contain numerous line ends (\\n) which force the message
 * to be sent as distinct messages.
 *
 * For example, the below are equivalent
 * \code
 *		say("Hello\nHow are you?") -- outputs the same as the below two
 *		say("Hello")
 *		say("How are you?")
 *	\endcode
 *
 * All messages support placeholders for player names. To insert a player's
 * name into a message include '{\<memory id\>}' somewhere.
 *
 * For example, the following code will output "Welcome to player New001"
 * if player_id is the id for a player named New001.
 * \code
 *		say("Welcome to player {" . player_id . "}!)
 *	\endcode
 *
 * The purpose of this is to fix broken player names from previous versions.
 *
 *	\addtogroup PhasorAPI
 *	@{
 */

#include "../phasor-lua.hpp"

/*! \brief Outputs a string to the server console.
 * \param str The string to print.
 *
 * Example usage:
 * \code
 *		hprintf("This will be printed to the console.")
 * \endcode
 */
int l_hprintf(lua_State* L);

/*! \brief Sends a chat message to the entire server.
 *
 * \param str The message to send.
 * \param [prepend] boolean indicating whether or not to prepend ** SERVER ** (default true)
 *
 * Example usage:
 * \code
 *		say("This is a single line message.")
 *		say("This is the first line of another.\nThis is the second.\nAnd so on")
 *		say("Don't include ** SERVER **", false)
 *	\endcode
 */
int l_say(lua_State* L);

/*! \brief Sends a chat message to the specified player.
 *
 * \param player_id The memory id of the player to message.
 * \param str The message to send.
 * \param [prepend] boolean indicating whether or not to prepend ** SERVER ** (default true)
 *
 * Example usage:
 * \code
 *		privatesay(1, "Hello") -- message player with id 1
 *		privatesay(1, "Hello", false) -- don't include ** SERVER **
 *	\endcode
 */
int l_privatesay(lua_State* L);

/*! \brief Sends the specified player a console message.
 *
 * \param player_id The memory id of the player to message.
 * \param str The message to send.
 *
 * Example usage:
 * \code
 *		sendconsoletext(4, "Your command was successful.")
 * \endcode
 */
int l_sendconsoletext(lua_State* L);

/*! \brief Responds to the person executing the current command.
 *
 * If the server console is executing it, this function acts like hprintf.
 *
 * If a player is executing it (via rcon), this function acts like sendconsoletext.
 *
 * \param str The message to send.
 *
 * Example usage:
 * \code
 *		respond("Your command was successfully executed.")
 *	\endcode
 */
int l_respond(lua_State* L);

/*! \brief Sends output to the specified logging stream.
 *
 * \param id The id of the logging stream to write to.
 * \param str The message to log.
 *
 * Valid log ids are as follows: \n
 *
 *    1 - Game log\n
 *    2 - Phasor log\n
 *    3 - Rcon log\n
 *    4 - Script log\n
 *
 * Example usage:
 * \code
 *		log_msg(1, "{4} won the game!")
 * \endcode
 *
 * The above example will output "<name of player 4> won the game!" to the
 * game log. If player 4 is called Oxide then "Oxide won the game" would be
 * saved.
 */
int l_log_msg(lua_State* L);

//! }@