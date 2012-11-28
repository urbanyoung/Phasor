#pragma once

#include <string>
#include <vector>
#include <stdarg.h>

std::string NarrowString(const std::wstring& wide);
std::wstring WidenString(const std::string& narrow);

std::string FormatVarArgs(const char* fmt, va_list marker);
std::wstring FormatVarArgsW(const wchar_t* fmt, va_list marker);
std::string m_sprintf(const char* _Format, ...);
std::wstring m_swprintf(const wchar_t* _Format, ...);

#define FORMATARGS(str, marker) \
	va_list ArgList; \
	va_start(ArgList, marker); \
	str = FormatVarArgs(marker, ArgList); \
	va_end(ArgList);

#define FORMATARGSW(str, marker) \
	va_list ArgList; \
	va_start(ArgList, marker); \
	str = FormatVarArgsW(marker, ArgList); \
	va_end(ArgList);

// Tokenization functions
std::vector<std::string> TokenizeCommand(const std::string& str);

template <class T>
std::vector<T> _TokenizeString(const T& str, const T& delim)
{
	// http://www.gamedev.net/community/forums/topic.asp?topic_id=381544#TokenizeString
	std::vector<T> tokens;
	size_t p0 = 0, p1 = T::npos;
	while (p0 != T::npos) {
		p1 = str.find_first_of(delim, p0);
		if(p1 != p0) {
			T token = str.substr(p0, p1 - p0);
			tokens.push_back(token);
		}
		p0 = str.find_first_not_of(delim, p1);
	}
	return tokens;
}