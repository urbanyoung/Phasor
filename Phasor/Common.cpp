#include "Common.h"
#include <sstream>
#include <stdio.h>

namespace Common
{
	// --------------------------------------------------------------------
	// Memory commands
	BOOL WriteBytes(DWORD dwAddress, LPVOID lpBuffer, DWORD dwCount)
	{
		BOOL bResult = TRUE;

		HANDLE hProcess = GetCurrentProcess();
		LPVOID lpAddress = UlongToPtr(dwAddress);

		DWORD dwOldProtect;
		DWORD dwNewProtect = PAGE_EXECUTE_READWRITE;

		bResult &= VirtualProtect(lpAddress, dwCount, dwNewProtect, &dwOldProtect);		// Allow read/write access
		bResult &= WriteProcessMemory(hProcess, lpAddress, lpBuffer, dwCount, NULL);	// Write to process memory
		bResult &= VirtualProtect(lpAddress, dwCount, dwOldProtect, &dwNewProtect);		// Restore original access
		bResult &= FlushInstructionCache(hProcess, lpAddress, dwCount);					// Update instruction cache

		return bResult;
	}

	BOOL ReadBytes(DWORD dwAddress, LPVOID lpBuffer, DWORD dwCount)
	{
		BOOL bResult = TRUE;

		HANDLE hProcess = GetCurrentProcess();
		LPVOID lpAddress = UlongToPtr(dwAddress);

		DWORD dwOldProtect;
		DWORD dwNewProtect = PAGE_EXECUTE_READWRITE;

		bResult &= VirtualProtect(lpAddress, dwCount, dwNewProtect, &dwOldProtect);	// Allow read/write access
		bResult &= ReadProcessMemory(hProcess, lpAddress, lpBuffer, dwCount, NULL);	// Reads from process memory
		bResult &= VirtualProtect(lpAddress, dwCount, dwOldProtect, &dwNewProtect);	// Restore original access

		return bResult;
	}

	// --------------------------------------------------------------------
	// String commands
	std::string NarrowString(std::wstring& str)
	{
		std::stringstream ss;
		for (size_t x = 0; x < str.length(); x++)
			ss << ss.narrow(str[x], '?');
		return ss.str();
	}

	std::wstring WidenString(std::string& str)
	{
		std::wstringstream ss;
		for (size_t x = 0; x < str.length(); x++)
			ss << ss.widen(str[x]);
		return ss.str();
	}

	// Format strings
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

	// Tokenize a string at spaces/quotation blocks
	std::vector<std::string> TokenizeCommand(const std::string& str)
	{
		std::vector<std::string> tokens;
		size_t len = str.size();

		if (len) {
			bool inQuote = false, data = false;
			std::stringstream curToken;

			for (int i = 0; i < len; i++) {
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

	// --------------------------------------------------------------------
	//
	// Windows error stuff
	// Formats the return value of GetLastError into a string
	void GetLastErrorAsText(std::string& out)
	{
		// FormatMessage will allocate this buffer with LocalAlloc
		LPVOID lpMsgBuf = 0;
		DWORD dw = GetLastError(); 

		DWORD dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS;
		FormatMessage(dwFlags, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	(LPTSTR)&lpMsgBuf, 0, NULL);

		if (lpMsgBuf) {
			out.assign((char*)lpMsgBuf);
			LocalFree(lpMsgBuf);
		} 
		else out.assign("No error");
	}
}