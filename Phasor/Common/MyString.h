#pragma once

#include <string>
#include <vector>
#include <stdarg.h>

// todo: cstr versions
std::string NarrowString(const std::wstring& wide);
std::wstring WidenString(const std::string& narrow);
// Remove all trailing \n characters from the input string.
std::wstring StripTrailingEndl(const std::wstring& str);
std::string StripTrailingEndl(const std::string& str);

void ToLowercase(std::string& str);
void ToLowercase(std::wstring& str);
void CStrToLower(char* str);

std::string FormatVarArgs(const char* fmt, va_list marker);
std::wstring FormatVarArgsW(const wchar_t* fmt, va_list marker);
std::string m_sprintf(const char* _Format, ...);
std::wstring m_swprintf(const wchar_t* _Format, ...);

//
// Number functions
// -----------------------------------------------------------------------
// Helper function for StringToNumber
template <class T>
T _StringToNumber(const char* start, char** end);

// Parse the input string as a number. If there are any characters that
// would produce a malformed number, false is returned.
template <class T>
bool StringToNumber(const std::string& str, T& out)
{
	const char* start = str.c_str(), *expected_end = start + str.size();
	char* end;
	T value = _StringToNumber<T>(start, &end);
	if (end != expected_end) return false;
	out = value;
	return true;
}

//
// Tokenization functions
// -----------------------------------------------------------------------

template <class T>
const T ArgsSearchString();
template <> const std::string ArgsSearchString<std::string>();
template <> const std::wstring ArgsSearchString<std::wstring>();

// Get the substring ending at the next occurrence of c.
// start is the position (inclusive) where to start searching from.
// end is the position after the next occurrence, or npos if none.
template <class T, class _Tc>
T GetStringEndingAtNext(const T& input, _Tc c, size_t start, size_t& end)
{
	size_t found = input.find_first_of(c, start);
	T out = input.substr(start, found - start);
	end = found == input.npos ? input.npos : found + 1;
	return out;
}

// Tokenize a string into its constituent arguments, an argument
// ends at a space unless within ' or " in which case it ends at the
// next escaping ' or ".
template <class T, class _Tc>
std::vector<T> TokenizeArgsT(const T& in)
{
	using namespace std;
	vector<T> out;
	const T tofind = ArgsSearchString<T>(); // " ' or space

	size_t curpos = 0;
	while (curpos != in.npos)
	{
		curpos = in.find_first_not_of(_Tc(' '), curpos);
		if (curpos == in.npos) break;
		size_t nextpos = in.find_first_of(tofind, curpos);
		if (nextpos == in.npos) { // no more matches, copy everything.
			out.push_back(in.substr(curpos, in.npos));
			break;
		}
		_Tc c = in.at(nextpos);
		size_t startfrom = c == _Tc(' ') ? curpos : curpos + 1;
		T token = GetStringEndingAtNext<T, _Tc>(in, c, startfrom, curpos);	
		if (token.size()) out.push_back(token);
	}
	return out;
}

typedef std::vector<std::string> (*tokargs_t)(const std::string&);
typedef std::vector<std::wstring> (*tokargsw_t)(const std::wstring&);
tokargs_t const TokenizeArgs = &TokenizeArgsT<std::string, char>;
tokargsw_t const TokenizeWArgs = &TokenizeArgsT<std::wstring, wchar_t>;

template <class T>
std::vector<T> Tokenize(const T& str, const T& delim)
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