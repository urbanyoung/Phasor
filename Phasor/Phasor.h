#pragma once

#include <string>
#include "Common.h"

namespace Phasor
{
	class Error
	{
	private:
		std::string err;
		bool hasErr;

	protected:
		void SetError(const std::string& error);

	public:
		Error();
		~Error();
		std::string GetError() const;
		bool hasError() const;
	};

	// Logs the error and then closes the server
	void AbortableError();

	// --------------------------------------------------------------------
	// 
	// Directory stuff
	bool SetupDirectories();
	std::string GetMapDirectory();
	std::string GetWorkingDirectory();
	std::string GetDataDirectory();
	std::string GetScriptDirectory();

	
}