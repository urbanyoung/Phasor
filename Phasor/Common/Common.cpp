#include "Common.h"
#include <sstream>
#include <stdio.h>
#include <iterator>

namespace Common
{
	// --------------------------------------------------------------------
	// Class: Object
	// Provides an interface between Lua and Phasor objects. The derived
	// classes provide specific types.
	// 
	Object::Object(obj_type _type) : type(_type) 
	{
	}

	Object::Object() : type(TYPE_NIL)
	{
	}

	Object::~Object() 
	{
	}

	std::unique_ptr<Object> Object::NewCopy() const
	{
		return std::unique_ptr<Object>(new Object(TYPE_NIL));
	}

	obj_type Object::GetType() const
	{
		return type;
	}

	// --------------------------------------------------------------------
	//

	ObjBool::ObjBool(bool b) : Object(TYPE_BOOL), b(b)
	{
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

	/*ObjString::ObjString(const char* str) : Object(TYPE_STRING)
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
	}*/


	// --------------------------------------------------------------------
	//
	ObjBlob::ObjBlob(BYTE* data, size_t size) : Object(TYPE_BLOB)
	{
		std::copy(data, data + size, std::back_inserter(this->data));
	}

	ObjBlob::ObjBlob(const ObjBlob& other) : Object(TYPE_BLOB)
	{
		this->data = other.data;
	}

	BYTE* ObjBlob::GetData(size_t& size)
	{
		size = data.size();
		return data.data();
	}

	std::unique_ptr<Object> ObjBlob::NewCopy() const
	{
		return std::unique_ptr<Object>(new ObjBlob(*this));
	}

	// --------------------------------------------------------------------
	//
	ObjTable::ObjTable(const std::map<std::string, std::string>& table) 
		: Object(TYPE_TABLE)
	{
		using namespace std;
		auto itr = table.cbegin();
		while (itr != table.end())
		{
			Object::unique_ptr key(new ObjString(itr->first.c_str()));
			Object::unique_ptr value(new ObjString(itr->second.c_str()));
			this->table.insert(pair_t(move(key), move(value)));
			itr++;
		}
	}

	ObjTable::ObjTable(const std::map<std::string, Object::unique_ptr>& table)
		: Object(TYPE_TABLE)
	{
		auto itr = table.cbegin();
		while (itr != table.end())
		{
			Object::unique_ptr key(new ObjString(itr->first.c_str()));
			Object::unique_ptr value(itr->second->NewCopy());
			this->table.insert(pair_t(move(key), move(value)));
			itr++;
		}
	}

/*	ObjTable::ObjTable(const table_t& table) 
		: Object(TYPE_TABLE)
	{
		//this->table = table;
		//table.clear();
	}*/

	ObjTable::ObjTable(const ObjTable& other)
		: Object(TYPE_TABLE)
	{
		CopyTable(other);
	}
		
	ObjTable::~ObjTable() 
	{
		FreeTable();
	}

	void ObjTable::FreeTable()
	{
		table.clear();
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
		auto itr = other.table.begin();
		while (itr != other.table.end())
		{
			table.insert(pair_t(
				itr->first->NewCopy(), itr->second->NewCopy()));
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
	std::vector<DWORD> FindSignatures(LPBYTE sigBuffer, LPBYTE sigWildCard,
		DWORD sigSize, LPBYTE pBuffer, DWORD size)
	{
		// thanks to Drew Benton
		std::vector<DWORD> results;
		for(DWORD index = 0; index < size; ++index)	{
			bool found = true;
			for(DWORD sindex = 0; sindex < sigSize; ++sindex) {
				// Make sure we don't overrun the buffer!
				if(sindex + index >= size) {
					found = false;
					break;
				}

				if(sigWildCard != 0) {
					if(pBuffer[index + sindex] != sigBuffer[sindex] &&
						sigWildCard[sindex] == 0) {
						found = false;
						break;
					}
				}
				else {
					if(pBuffer[index + sindex] != sigBuffer[sindex]) {
						found = false;
						break;
					}
				}
			}

			if(found) results.push_back(index);
		}
		return results;
	}

	bool FindSignature(LPBYTE sigBuffer, LPBYTE sigWildCard, 
		DWORD sigSize, LPBYTE pBuffer, DWORD size, DWORD occurance,
		DWORD& result)
	{
		bool success = false;
		DWORD count = 0;
		for(DWORD index = 0; index < size; ++index)	{
			bool found = true;
			for(DWORD sindex = 0; sindex < sigSize; ++sindex) {
				// Make sure we don't overrun the buffer!
				if(sindex + index >= size) {
					found = false;
					break;
				}

				if(sigWildCard != 0) {
					if(pBuffer[index + sindex] != sigBuffer[sindex] &&
						sigWildCard[sindex] == 0) {
							found = false;
							break;
					}
				}
				else {
					if(pBuffer[index + sindex] != sigBuffer[sindex]) {
						found = false;
						break;
					}
				}
			}

			if(found) {
				if (occurance == count) {
					result = index;
					success = true;
					break;
				}
				count++;
			}
		}
		return success;
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