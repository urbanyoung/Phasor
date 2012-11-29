#pragma once

#include <set>
#include <string>

namespace Scripting
{
	class PhasorScript
	{
	private:
		std::set<std::string> blockedFunctions;
		std::string name, path;

	public:
		virtual ~PhasorScript() {}

		void BlockFunction(const std::string& func)
		{
			blockedFunctions.insert(func);
		}

		void SetInfo(const std::string& path, const std::string& name)
		{
			this->name = name;
			this->path = path;
		}

		const std::string& GetName()
		{
			return name;
		}

		const std::string& GetPath()
		{
			return path;
		}

		// Checks if the specified script function is allowed to be called.
		bool FunctionAllowed(const std::string& func)
		{
			return blockedFunctions.find(func) == blockedFunctions.end();
		}
	};
}