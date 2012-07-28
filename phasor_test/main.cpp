#include <windows.h>
#include <string>
#include <stdio.h>

// Formats a va_list and return a std::string representation
std::string FormatVarArgs(const char* _Format, va_list ArgList)
{
	int count = _vscprintf(_Format, ArgList);
	if (count < 1024) {
		char szText[1024];
		_vsnprintf_s(szText, sizeof(szText)/sizeof(szText[0]),
			_TRUNCATE, _Format, ArgList);
		return std::string(szText);
	} else {
		try { // would rather return a junk message than crash
			char* buf = new char[count + 1];			
			_vsnprintf_s(buf, count+1, _TRUNCATE, _Format, ArgList);
			std::string str(buf, count);
			delete[] buf;
			return str;
		} catch (std::bad_alloc) {
			return std::string("std::bad_alloc on FormatVarArgs");
		}
	}
}

std::wstring WFormatVarArgs(const wchar_t* _Format, va_list ArgList)
{
	int count = _vscwprintf(_Format, ArgList);

	if (count < 1024) {
		wchar_t szText[1024];
		_vsnwprintf_s(szText, sizeof(szText)/sizeof(szText[0]),
			_TRUNCATE, _Format, ArgList);
		return std::wstring(szText);
	} else {
		try { // would rather return a junk message than crash
			wchar_t* buf = new wchar_t[count + 1];			
			_vsnwprintf_s(buf, count+1, _TRUNCATE, _Format, ArgList);
			std::wstring str(buf, count);
			delete[] buf;
			return str;
		} catch (std::bad_alloc) {
			return std::wstring(L"std::bad_alloc on WFormatVarArgs");
		}
	}
}

std::string m_sprintf_s(const char* _Format, ...)
{
	va_list ArgList;
	va_start(ArgList, _Format);
	std::string str = FormatVarArgs(_Format, ArgList);
	va_end(ArgList);
	return str;
}

std::wstring m_swprintf_s(const wchar_t* _Format, ...)
{
	va_list ArgList;
	va_start(ArgList, _Format);
	std::wstring str = WFormatVarArgs(_Format, ArgList);
	va_end(ArgList);
	return str;
}

int main()
{
	size_t len = 1024;
	wchar_t* str = new wchar_t[len];
	for (int i = 0; i < len-1; i++)
		str[i] = L'1';
	str[len-2] = L'a';
	str[len-1] = 0;
	wprintf(L"%s\n", str);

	std::wstring t = m_swprintf_s(L"%s", str);
	wprintf(L"%s\n", t.c_str());
	return 0;
}