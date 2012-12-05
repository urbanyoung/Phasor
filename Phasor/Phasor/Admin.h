#pragma once

#include <string>
#include "../Common/Streams.h"

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
	result_t Add(const std::string& hash, const std::string& authname, int level);

	// Removes an existing admin, does nothing if they're not an admin.
	void Remove(const std::string& hash);

	// Checks if a player is an admin
	bool IsAdmin(const std::string& hash);

	// Checks if a player can execute the specified command
	// Returns E_OK if there are no admins (the system is inactive)
	result_t CanUseCommand(const std::string& hash, const std::string& command);

	// Initialize the admin system, outputting errors to 'out'.
	// Errors aren't output if 'out' is NULL.
	void Initialize(COutStream* out);

	// Reloads the admin/access files
	void Reload();
}