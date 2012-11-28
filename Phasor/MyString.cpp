#include "MyString.h"
#include <sstream>

std::string NarrowString(const std::wstring& str)
{
	std::stringstream ss;
	for (size_t x = 0; x < str.length(); x++)
		ss << ss.narrow((char)str[x], '?');
	return ss.str();
}

std::wstring WidenString(const std::string& str)
{
	std::wstringstream ss;
	for (size_t x = 0; x < str.length(); x++)
		ss << ss.widen(str[x]);
	return ss.str();
}