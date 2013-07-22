#pragma once

#include <string>
#include "../Common/Streams.h"

enum e_command_result;

namespace commands
{
	class CArgParser;
}

namespace Admin
{
	enum result_t
	{
		E_OK = 0,
		E_HASH_INUSE,
		E_LEVEL_NOT_EXIST,
		E_NAME_INUSE,
		E_NOT_ADMIN,
		E_NOT_ALLOWED,
	};

	// Attempts to add the the admin to the list
	result_t add(const std::string& hash, const std::string& authname, int level);

	// Removes an existing admin, does nothing if they're not an admin.
	void remove(const std::string& hash);

	// Checks if a player is an admin
	bool isAdmin(const std::string& hash);

	// Gets an admin's access level.
	bool getLevel(const std::string& hash, int* level);

	// Checks if hash challenging is enabled (defaults to true)
	bool isChallengeEnabled();

	// Checks if a player can execute the specified command
	// Returns E_OK if there are no admins (the system is inactive)
	result_t canUseCommand(const std::string& hash, const std::string& command,
		std::string* authName=NULL);

	// Initialize the admin system, outputting errors to 'out'.
	// Errors aren't output if 'out' is NULL.
	void initialize(COutStream* out);

	// Reloads the admin/access files
	void reload();

	// -------------------------------------------------------------------
	//
	/*! \brief Make the specified player (or hash) an admin.
	 * 
	 *	\param player_or_hash Either the player's in game id or their hash.
	 *	\param authname The name to auth the player under.
	 *	\param level The level the player should have access to.
	 *
	 *	Example usage: sv_admin_add 1 Oxide 0
	 */
	e_command_result sv_admin_add(void*, commands::CArgParser& args, COutStream& out);

	/*! \brief Revoke a current admin's status.
	 *
	 *	\param name The name of the admin to remove.
	 *
	 *	Example usage: sv_admin_del Oxide
	 */
	e_command_result sv_admin_del(void*, commands::CArgParser& args, COutStream& out);

	/*! \brief Lists all admins.
	 *
	 *	Example usage: sv_admin_list
	 */
	e_command_result sv_admin_list(void*, commands::CArgParser& args, COutStream& out);

	/*! \brief Lists all admins who are currently in the server.
	 *
	 *	Example usage: sv_admin_cur
	 */
	e_command_result sv_admin_cur(void*, commands::CArgParser& args, COutStream& out);

	/*! \brief Reload both the access and admin files.
	 *
	 *	Example usage: sv_admin_reload
	 */
	e_command_result sv_admin_reload(void*, commands::CArgParser& args, COutStream& out);

	/*! \brief Lists the commands you have permission to use.
	 *
	 *	Example usage: sv_commands
	 */
	e_command_result sv_commands(void* player, commands::CArgParser& args, COutStream& out);

	/*! \brief Enables/Disables forced-hash checking. This protects against people
	 *	attempting to steal an admin's hash, an admin won't be marked as such until
	 *	the response from gamespy is received.
	 */
	e_command_result sv_admin_check(void*, commands::CArgParser& args, COutStream& out);
	e_command_result sv_public(void*, commands::CArgParser& args, COutStream& out);
}