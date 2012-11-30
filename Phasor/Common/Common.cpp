#include "Common.h"
#include <sstream>
#include <stdio.h>

namespace Common
{
	// --------------------------------------------------------------------
	// Class: Object
	// Provides an interface between Lua and Phasor objects. The derived
	// classes provide specific types.
	// 
	Object::Object(obj_type _type) : type(_type) {}

	Object::Object() : type(TYPE_NIL) {}

	Object::~Object() {}

	std::unique_ptr<Object> Object::NewCopy() const
	{
		return std::unique_ptr<Object>(new Object(TYPE_NIL));
	}

	obj_type Object::GetType() const
	{
		return type;
	}

	std::stringstream Object::ConversionDesc(obj_type totype) const
	{
		std::stringstream err;
		err << "object: no possible conversion from '" << obj_desc[type]
		<< "' to '" << obj_desc[totype] << "'";
		return err;
	}

	// --------------------------------------------------------------------
	//

	ObjBool::ObjBool(bool b) : Object(TYPE_BOOL)
	{
		this->b = b;
	}

	ObjBool::ObjBool(const ObjBool& other) : Object(TYPE_BOOL)
	{
		this->b = other.b;
	}

	ObjBool::~ObjBool()
	{
	}

	ObjBool & ObjBool::operator=(const ObjBool &rhs) 
	{
		if (this == &rhs) return *this;

		this->b = rhs.b;
		return *this;
	}

	std::unique_ptr<Object> ObjBool::NewCopy() const
	{
		return std::unique_ptr<Object>(new ObjBool(*this));
	}

	bool ObjBool::GetValue() const
	{
		return this->b;
	}

	// --------------------------------------------------------------------
	//

	ObjNumber::ObjNumber(int value) : Object(TYPE_NUMBER)
	{
		this->value = value;
	}

	ObjNumber::ObjNumber(DWORD value) : Object(TYPE_NUMBER)
	{
		this->value = value;
	}

	ObjNumber::ObjNumber(double value) : Object(TYPE_NUMBER)
	{
		this->value = value;
	}

	ObjNumber::ObjNumber(float value) : Object(TYPE_NUMBER)
	{
		this->value = value;
	}

	ObjNumber::~ObjNumber()
	{
	}

	ObjNumber::ObjNumber(const ObjNumber& other) : Object(TYPE_NUMBER)
	{
		this->value = other.value;
	}

	ObjNumber& ObjNumber::operator=(const ObjNumber& rhs)
	{
		if (this == &rhs) return *this;

		this->value = rhs.value;
		return *this;
	}

	std::unique_ptr<Object> ObjNumber::NewCopy() const
	{
		return std::unique_ptr<Object>(new ObjNumber(*this));
	}

	double ObjNumber::GetValue() const
	{
		return this->value;
	}

	// --------------------------------------------------------------------
	//

	ObjString::ObjString(const char* str) : Object(TYPE_STRING)
	{
		CopyString(str);
	}

	ObjString::ObjString(const ObjString& other) : Object(TYPE_STRING)
	{
		CopyString(other.str);
	}

	ObjString::~ObjString()
	{
		delete[] str;
	}

	ObjString& ObjString::operator=(const ObjString& rhs) 
	{
		if (this == &rhs) return *this;

		delete[] str;
		CopyString(rhs.str);

		return *this;
	}

	std::unique_ptr<Object> ObjString::NewCopy() const
	{
		return std::unique_ptr<Object>(new ObjString(*this));
	}

	void ObjString::CopyString(const char* str)
	{
		this->str = new char [strlen(str) + 1];
		strcpy(this->str, str);
	}

	const char* ObjString::GetValue() const
	{
		return this->str;
	}

	// --------------------------------------------------------------------
	//

	ObjTable::ObjTable(const std::map<std::string, std::string>& table) : Object(TYPE_TABLE)
	{
		using namespace std;
		map<string, string>::const_iterator itr = table.begin();
		while (itr != table.end())
		{
			ObjString* key = new ObjString(itr->first.c_str());
			ObjString* value = new ObjString(itr->second.c_str());
			this->table.insert(pair<Object*, Object*>(key, value));
			itr++;
		}
	}

	ObjTable::ObjTable(std::map<Object*, Object*>& table)
	{
		this->table = table;
		table.clear();
	}

	ObjTable::ObjTable(const ObjTable& other) : Object(TYPE_TABLE)
	{
		CopyTable(other);
	}
		
	ObjTable::~ObjTable() 
	{
		FreeTable();
	}

	void ObjTable::FreeTable()
	{
		std::map<Object*, Object*>::iterator itr = table.begin();
		while (itr != table.end())
		{
			delete itr->first;
			delete itr->second;
			itr = table.erase(itr);
		}
	}

	ObjTable& ObjTable::operator=(const ObjTable &rhs)
	{
		if (this == &rhs) return *this;
		FreeTable();
		CopyTable(rhs);
		return *this;
	}

	std::unique_ptr<Object> ObjTable::NewCopy() const
	{
		return std::unique_ptr<Object>(new ObjTable(*this));
	}

	void ObjTable::CopyTable(const ObjTable& other)
	{
		std::map<Object*, Object*>::const_iterator itr = other.table.begin();

		while (itr != other.table.end())
		{
			Object* key = itr->first->NewCopy().release();
			Object* value = itr->second->NewCopy().release();
			table.insert(std::pair<Object*, Object*>(key, value));
		}
	}

	/*const Object& ObjTable::operator [] (const Object& key)
	{
		std::map<Object*, Object*>::iterator itr = table.find((Object*)&key);
		if (itr == table.end()) {
			std::stringstream err;
			err << __FUNCTION__ << ": specified key doesn't exist ";
			throw std::exception(err.str().c_str());
		}
		return itr->second;
	}*/

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