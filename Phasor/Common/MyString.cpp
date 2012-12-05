#include "MyString.h"
#include <sstream>
#include <algorithm>

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

void ToLowercase(std::string& str)
{
	std::transform(str.begin(), str.end(),str.begin(), tolower);
}

std::string FormatVarArgs(const char* fmt, va_list marker)
{
	// http://www.codeproject.com/Articles/15115/How-to-Format-a-String
	using std::string;
	using std::vector;
	string retStr;
	if (fmt) {
		// Get formatted string length adding one for NULL
		size_t len = _vscprintf(fmt, marker) + 1;

		// Create a char vector to hold the formatted string.
		vector<char> buffer(len, '\0');
		int nWritten = _vsnprintf_s(&buffer[0], buffer.size(), len, fmt, marker);   
		if (nWritten > 0) retStr = &buffer[0];
	}

	return retStr;
}

std::wstring FormatVarArgsW(const wchar_t* fmt, va_list marker)
{
	// http://www.codeproject.com/Articles/15115/How-to-Format-a-String
	using std::wstring;
	using std::vector;
	wstring retStr;
	if (fmt) {
		// Get formatted string length adding one for NULL
		size_t len = _vscwprintf(fmt, marker) + 1;

		// Create a char vector to hold the formatted string.
		vector<wchar_t> buffer(len, '\0');
		int nWritten = _vsnwprintf_s(&buffer[0], buffer.size(), len, fmt, marker);   
		if (nWritten > 0) retStr = &buffer[0];
	}

	return retStr;
}

std::wstring m_swprintf(const wchar_t *fmt, ...) 
{ 
	va_list ArgList;
	va_start(ArgList, fmt);
	std::wstring str = FormatVarArgsW(fmt, ArgList);
	va_end(ArgList);
	return str;
}

std::string m_sprintf(const char *fmt, ...) 
{ 
	va_list ArgList;
	va_start(ArgList, fmt);
	std::string str = FormatVarArgs(fmt, ArgList);
	va_end(ArgList);
	return str;
}

// Used in TokenizeArgs
template <>
const std::string ArgsSearchString<std::string>()
{
	return std::string("\"' ");
}

// Used in TokenizeArgs
template <>
const std::wstring ArgsSearchString<std::wstring>()
{
	return std::wstring(L"\"' ");
}
