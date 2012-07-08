#include "Common.h"
#include <sstream>

namespace common 
{
	std::string NarrowString(std::wstring str)
	{
		std::stringstream ss;
		for (size_t x = 0; x < str.length(); x++)
			ss << ss.narrow(str[x], '?');
		return ss.str();
	}

	std::wstring WidenString(std::string str)
	{
		std::wstringstream ss;
		for (size_t x = 0; x < str.length(); x++)
			ss << ss.widen(str[x]);
		return ss.str();
	}
}