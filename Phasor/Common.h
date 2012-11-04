#pragma once
#pragma warning( disable : 4290 )

#include <windows.h>
#include <string>
#include <vector>
#include <map>

namespace Common
{
	// Type of data stored by an Object
	enum obj_type 
	{
		TYPE_NIL = 0, // undefined type
		TYPE_BOOL,
		TYPE_NUMBER,
		TYPE_STRING,
		TYPE_TABLE
	};

	// --------------------------------------------------------------------
	// Class: Object
	// Provides an interface between Lua and Phasor objects. The derived
	// classes provide specific types.
	class Object
	{
	private: 
		obj_type type;

	protected:

		// Should be called by derived classes to set the object type.
		Object(obj_type type);

	public:
		// Creates a nil object
		Object();
		virtual ~Object();

		// Create a new, independent copy of this object.
		virtual Object* NewCopy() const;

		// Returns the type of this object
		obj_type GetType() const;
	};

	// --------------------------------------------------------------------
	// Class: ObjBool
	// Wrapper for a boolean type
	class ObjBool : public Object
	{
	private:
		bool b;

	public:
		ObjBool(bool b);
		~ObjBool();

		ObjBool & operator=(const ObjBool &rhs); 
		ObjBool(const ObjBool& other );	

		// Create a new, independent copy of this object.
		virtual ObjBool* NewCopy() const;

		// Return the value stored in this object.
		bool GetValue() const;
	};

	// --------------------------------------------------------------------
	// Class: ObjNumber
	// Wrapper for a number type
	class ObjNumber : public Object
	{
	private:
		double value;

	public: 
		ObjNumber(int value);
		ObjNumber(DWORD value);
		ObjNumber(double value);
		ObjNumber(float value);
		~ObjNumber();

		ObjNumber & operator=(const ObjNumber &rhs); 
		ObjNumber(const ObjNumber& other );

		// Create a new, independent copy of this object.
		virtual ObjNumber* NewCopy() const;

		// Return the value stored in this object
		double GetValue() const;
	};

	// --------------------------------------------------------------------
	// Class: ObjString
	// Wrapper for a string type
	class ObjString  : public Object
	{
	private:
		char* str;

		// Copies str into this object.
		void CopyString(const char* str);

	public:
		ObjString(const char* str);
		~ObjString();

		ObjString & operator=(const ObjString &rhs);
		ObjString(const ObjString& other );

		// Create a new, independent copy of this object.
		virtual ObjString* NewCopy() const;

		// Return the value stored in this object
		const char* GetValue() const;
	};

	// --------------------------------------------------------------------
	// Class: ObjTable
	// Wrapper for an associative container
	class ObjTable : public Object
	{
	private:
		std::map<Object*, Object*> table;

		// Copies other into this object
		void CopyTable(const ObjTable& other);

		// Frees the table associated with this object
		void FreeTable();

	public:
		ObjTable(const std::map<std::string, std::string>& table);
		ObjTable(std::map<Object*, Object*>& table);
		~ObjTable();

		ObjTable & operator=(const ObjTable &rhs);
		ObjTable(const ObjTable& other );

		// Create a new, independent copy of this object.
		virtual ObjTable* NewCopy() const;

		// Get value at index
		//const Object* operator [] (size_t i);
		// Returns the value associated with the specified key
		const Object* operator [] (const Object& key);
	};

	// --------------------------------------------------------------------
	// Memory commands
	BOOL WriteBytes(DWORD dwAddress, LPVOID lpBuffer, DWORD dwCount);
	BOOL ReadBytes(DWORD dwAddress, LPVOID lpBuffer, DWORD dwCount);
	std::vector<DWORD> FindSignature(LPBYTE lpBuffer, DWORD dwBufferSize, LPBYTE lpSignature, DWORD dwSignatureSize, LPBYTE lpWildCards = 0);
	DWORD FindAddress(LPBYTE lpBuffer, DWORD dwBufferSize, LPBYTE lpSignature, DWORD dwSignatureSize, LPBYTE lpWildCards = 0, DWORD dwIndex = 0, DWORD dwOffset = 0);
	BOOL CreateCodeCave(DWORD dwAddress, BYTE cbSize, VOID (*pFunction)());

	// --------------------------------------------------------------------
	// String commands
	std::string NarrowString(std::wstring&);
	std::wstring WidenString(std::string&);

	// Format strings
	std::string FormatVarArgs(const char* _Format, va_list ArgList);
	std::wstring WFormatVarArgs(const wchar_t* _Format, va_list ArgList);
	std::string m_sprintf_s(const char* _Format, ...);
	std::wstring m_swprintf_s(const wchar_t* _Format, ...);
	
	// Tokenization functions
	std::vector<std::string> TokenizeCommand(const std::string& str);

	template <class T>
	std::vector<T> TokenizeString(const T& str, const T& delim)
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
	
	// --------------------------------------------------------------------
	//
	// Windows error stuff
	// Formats the return value of GetLastError into a string
	void GetLastErrorAsText(std::string& out);
}
