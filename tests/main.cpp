#include <string>
#include <stdio.h>
#include <stdarg.h>

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

int main()
{

}