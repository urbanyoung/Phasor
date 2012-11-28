#pragma once

#include <set>
#include <string>

namespace Scripting
{
	class PhasorScript
	{
	private:
		std::set<std::string> blockedFunctions;

	public:
		virtual ~PhasorScript() {}

		void BlockFunction(const std::string& func)
		{
			blockedFunctions.insert(func);
		}

		// Checks if the specified script function is allowed to be called.
		bool FunctionAllowed(const std::string& func)
		{
			return blockedFunctions.find(func) == blockedFunctions.end();
		}
	};
}