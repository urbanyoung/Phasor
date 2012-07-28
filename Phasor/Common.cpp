#include "Common.h"
#include <sstream>
#include <stdio.h>

namespace Common
{
	//-----------------------------------------------------------------------------------------
	// Class: ObjectError
	//
	ObjectError::ObjectError(int err, const char* desc) : std::exception() 
	{
		this->err = err;
		processed = true; // assume we're processing it
		switch (err)
		{
		case OBJECT_TYPE:
			msg.assign("ObjectError - Attempted to access unexpected data type.");
			break;

		default:
			processed = false; // not processed
		}		

		if (processed && desc) 
			msg += "\ndescription: " + std::string(desc);
	}

	ObjectError::~ObjectError()
	{
	}

	const char* ObjectError::what() const throw()
	{ 
		return msg.c_str();
	}

	//-----------------------------------------------------------------------------------------
	// Class: ObjectWrap
	//
	int created = 0;
	ObjectWrap::c_data::c_data() : size(0) {
		created++;
		type = TYPE_NONE;
	}
	ObjectWrap::c_data::c_data(const char* val) : size(4) {
		created++;
		pdata.s = new std::string;
		pdata.s->assign(val);
		type = TYPE_STRING;
	}
	ObjectWrap::c_data::c_data(const wchar_t* val) : size(4) {
		created++;
		pdata.ws = new std::wstring;
		pdata.ws->assign(val);
		type = TYPE_WSTRING;
	}
	ObjectWrap::c_data::c_data(int val) : size(4) {
		created++;
		pdata.i = new int;
		*pdata.i = val;
		type = TYPE_INT;
	}
	ObjectWrap::c_data::c_data(double val) : size(4) {
		created++;
		pdata.d = new double;
		*pdata.d = val;
		type = TYPE_DOUBLE;			
	}
	ObjectWrap::c_data::c_data(BYTE* val, size_t length) : size(length) { 
		created++;
		pdata.b = new BYTE[length];
		memcpy(pdata.b, val, length);
		type = TYPE_BLOB;
	}
	ObjectWrap::c_data::c_data(void* val) : size(4) {
		created++;
		pdata.ptr = new BYTE*;
		*pdata.ptr = (BYTE*)val;
		type = TYPE_PTR;
	}

	ObjectWrap::c_data::~c_data()
	{
		created--;
		printf("%i open objects\n", created);
		switch (type)
		{
		case TYPE_INT:
			delete pdata.i;
			break;		
		case TYPE_STRING:
			delete pdata.s;
			break;
		case TYPE_WSTRING:
			delete pdata.ws;
			break;
		case TYPE_DOUBLE:
			delete pdata.d;
			break;
		case TYPE_BLOB:
			delete[] pdata.b;
			break;
		case TYPE_PTR:
			delete pdata.ptr;
			break;
		} 
	}

	ObjectWrap::ObjectWrap() : data() 
	{

	}

	ObjectWrap::ObjectWrap(const char* val) : data(new c_data(val))
	{		
	}

	ObjectWrap::ObjectWrap(const wchar_t* val) : data(new c_data(val))
	{		
	}

	ObjectWrap::ObjectWrap(int val) : data(new c_data(val))
	{		
	}

	ObjectWrap::ObjectWrap(double val) : data(new c_data(val))
	{		
	}

	ObjectWrap::ObjectWrap(BYTE* val, size_t length) : data(new c_data(val, length))
	{		
	}

	ObjectWrap::ObjectWrap(const ObjectWrap &v) : data(v.data)
	{		
	}

	ObjectWrap::ObjectWrap(void* ptr) : data(new c_data(ptr))
	{
	}

	ObjectWrap& ObjectWrap::operator= (const ObjectWrap &v)
	{
		//printf("%08X %08X\n", v, this);
		data = v.data;
		return *this;
	}

	ObjectWrap::~ObjectWrap()
	{
		printf("Destroy wrapper\n");
		//printf("Destruct type %i\n", data->type);
	}

	std::string ObjectWrap::GetStr() const throw(ObjectError)
	{
		// let strings convert themselves
		if (data->type == TYPE_WSTRING) return Common::NarrowString(*data->pdata.ws);
		VerifyType(TYPE_STRING);
		return *(data->pdata.s);
	}

	std::wstring ObjectWrap::GetWStr() const throw(ObjectError)
	{
		if (data->type == TYPE_STRING) return Common::WidenString(*data->pdata.s);
		VerifyType(TYPE_WSTRING);
		return *(data->pdata.ws);
	}

	int ObjectWrap::GetInt() const throw(ObjectError)
	{
		VerifyType(TYPE_INT);
		return *(data->pdata.i);
	}

	double ObjectWrap::GetDouble() const throw(ObjectError)
	{
		VerifyType(TYPE_DOUBLE);
		return *(data->pdata.d);
	}

	BYTE* ObjectWrap::GetBlob() const throw(ObjectError)
	{
		VerifyType(TYPE_BLOB);
		return data->pdata.b;
	}

	std::string ObjectWrap::ToString()
	{
		std::stringstream s;
		switch (data->type)
		{
		case TYPE_STRING:
			s << *data->pdata.s;
			break;
		case TYPE_WSTRING:
			s << Common::NarrowString(*data->pdata.ws);
			break;
		case TYPE_INT:
			s << *data->pdata.i;
			break;
		case TYPE_DOUBLE:
			s << *data->pdata.d;
			break;
		case TYPE_BLOB:
			s << data->size << " byte BLOB@" << (DWORD)data->pdata.b;
			break;
		}
		std::string str = s.str();
		return str;
	}

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

	// Finds all locations of a signature
	std::vector<DWORD> FindSignature(LPBYTE lpBuffer, DWORD dwBufferSize, LPBYTE lpSignature, DWORD dwSignatureSize, LPBYTE lpWildCards)
	{
		std::vector<DWORD> addresses;
		for (DWORD i = 0; i < dwBufferSize; ++i) {
			bool bFound = true;
			// Loop through each byte in the signature
			for (DWORD j = 0; j < dwSignatureSize; ++j)	{
				// Check if the index overruns the buffer
				if (i + j >= dwBufferSize){
					bFound = false;
					break;
				}

				if (lpWildCards) {
					// Check if the buffer does not equal the signature and a wild card is not set
					if (lpBuffer[i + j] != lpSignature[j] && !lpWildCards[j]){
						bFound = false;
						break;
					}
				}
				else {
					if (lpBuffer[i + j] != lpSignature[j]){
						bFound = false;
						break;
					}
				}
			}

			if (bFound)
				addresses.push_back(i);
		}

		return addresses;
	}

	// Finds the location of a signature
	DWORD FindAddress(LPBYTE lpBuffer, DWORD dwBufferSize, LPBYTE lpSignature, DWORD dwSignatureSize, LPBYTE lpWildCards, DWORD dwIndex, DWORD dwOffset)
	{
		DWORD dwAddress = 0;
		std::vector<DWORD> addresses = FindSignature(lpBuffer, dwBufferSize, lpSignature, dwSignatureSize, lpWildCards);
		if (addresses.size() - 1 >= dwIndex)
			dwAddress = (DWORD)lpBuffer + addresses[dwIndex] + dwOffset;

		return dwAddress;
	}

	// Creates a code cave to a function at a specific address
	BOOL CreateCodeCave(DWORD dwAddress, BYTE cbSize, VOID (*pFunction)())
	{
		BOOL bResult = TRUE;

		if (cbSize < 5)
			return FALSE;

		// Calculate the offset from the function to the address
		DWORD dwOffset = PtrToUlong(pFunction) - dwAddress - 5;

		// Construct the call instruction to the offset
		BYTE patch[0xFF] = {0x90};
		patch[0] = 0xE8;
		memcpy(patch + 1, &dwOffset, sizeof(dwAddress));

		// Write the code cave to the address
		bResult &= Common::WriteBytes(dwAddress, patch, cbSize);

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