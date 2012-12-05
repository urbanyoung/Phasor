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

	result_t Add(const std::string& hash, const std::string& authname, int level);
	void Remove(const std::string& hash);
	bool IsAdmin(const std::string& hash);
	result_t CanUseCommand(const std::string& hash, const std::string& command);

	void Initialize(COutStream* out);
	void Reload();
}