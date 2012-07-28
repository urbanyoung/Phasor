#pragma once

#include <windows.h>
#include <string>
#include <vector>

namespace Common
{
	// --------------------------------------------------------------------
	// Memory commands
	BOOL WriteBytes(DWORD dwAddress, LPVOID lpBuffer, DWORD dwCount);
	BOOL ReadBytes(DWORD dwAddress, LPVOID lpBuffer, DWORD dwCount);

	// --------------------------------------------------------------------
	// String commands
	std::string NarrowString(std::wstring&);
	std::wstring WidenString(std::string&);

	// Format strings
	std::string FormatVarArgs(const char* _Format, va_list ArgList);
	std::wstring WFormatVarArgs(const wchar_t* _Format, va_list ArgList);
	std::string m_sprintf_s(const char* _Format, ...);
	std::wstring m_swprintf_s(const wchar_t* _Format, ...);
	
	template <class T>
	std::vector<T> TokenizeString(const T& str, const T& delim)
	{
		// http://www.gamedev.net/community/forums/topic.asp?topic_id=381544#TokenizeString
		std::vector<T> tokens;
		size_t p0 = 0, p1 = T::npos;
		while (p0 != T::npos) {
			p1 = str.find_first_of(delim, p0);
			if(p1 != p0) {
				wstring token = str.substr(p0, p1 - p0);
				tokens.push_back(token);
			}
			p0 = str.find_first_not_of(delim, p1);
		}
		return tokens;
	}

	std::vector<std::string> TokenizeCommand(const std::string& str);

	// --------------------------------------------------------------------
	//
	// Windows error stuff
	// Formats the return value of GetLastError into a string
	void GetLastErrorAsText(std::string& out);
}
