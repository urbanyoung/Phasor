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

// Tokenize a string at spaces/quotation blocks
std::vector<std::string> TokenizeCommand(const std::string& str)
{
	std::vector<std::string> tokens;
	size_t len = str.size();

	if (len) {
		bool inQuote = false, data = false;
		std::stringstream curToken;

		for (size_t i = 0; i < len; i++) {
			if (str[i] == '"') {
				// If there's currently data save it
				if (data) {
					tokens.push_back(curToken.str());
					curToken.clear();
					data = false;
				}
				inQuote = !inQuote;
				continue;
			}

			// If in quote append data regardless of what it is
			if (inQuote) {
				curToken << str[i];
				data = true;
			}
			else {
				if (str[i] == ' ' && data) {
					tokens.push_back(curToken.str());
					curToken.clear();
					data = false;
				}
				else {
					curToken << str[i];
					data = true;
				}
			}
		}

		if (data) tokens.push_back(curToken.str());
	}

	return tokens;
}
