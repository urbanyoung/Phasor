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
	std::wstring str;
	FORMATARGSW(str, fmt);
	return str;
}

std::string m_sprintf(const char *fmt, ...) 
{ 
	std::string str;
	FORMATARGS(str, fmt);
	return str;
}

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

std::vector<std::string> TokenizeArgs(const std::string& in)
{
	using namespace std;
	vector<string> out;
	const string tofind = "\"' "; // " ' or space

	size_t curpos = 0;
	while (curpos != in.npos)
	{
		curpos = in.find_first_not_of(' ', curpos);
		if (curpos == in.npos) break;
		size_t nextpos = in.find_first_of(tofind, curpos);
		if (nextpos == in.npos) { // no more matches, copy everything.
			out.push_back(in.substr(curpos, in.npos));
			break;
		}
		char c = in.at(nextpos);
		size_t startfrom = c == ' ' ? curpos : curpos + 1;
		string token = GetStringEndingAtNext<string, char>(in, c, startfrom, curpos);	
		if (token.size()) out.push_back(token);
	}
	return out;
}

std::vector<std::wstring> TokenizeWArgs(const std::wstring& in)
{
	using namespace std;
	vector<wstring> out;
	const wstring tofind = L"\"' "; // " ' or space

	size_t curpos = 0;
	while (curpos != in.npos)
	{
		curpos = in.find_first_not_of(' ', curpos);
		if (curpos == in.npos) break;
		size_t nextpos = in.find_first_of(tofind, curpos);
		if (nextpos == in.npos) { // no more matches, copy everything.
			out.push_back(in.substr(curpos, in.npos));
			break;
		}
		wchar_t c = in.at(nextpos);
		size_t startfrom = c == ' ' ? curpos : curpos + 1;
		wstring token = GetStringEndingAtNext<wstring, wchar_t>(in, c, startfrom, curpos);	
		if (token.size()) out.push_back(token);
	}
	return out;
}
