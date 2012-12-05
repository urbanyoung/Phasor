#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <deque>

namespace Common
{
	// Type of data stored by an Object
	enum obj_type 
	{
		TYPE_NIL = 0, // undefined type
		TYPE_BOOL,
		TYPE_NUMBER,
		TYPE_STRING,
		TYPE_TABLE,
		TYPE_BLOB
	};

	// String descriptions matching types in obj_type
	static const char* obj_desc[] = 
	{
		"nil",
		"bool",
		"number",
		"string",
		"table"
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

		Object& operator=(const Object &rhs); 
		Object(const Object& other);

	public:
		typedef std::unique_ptr<Object> unique_ptr;
		typedef std::list<unique_ptr> unique_list;
		typedef std::deque<unique_ptr> unique_deque;

		// Creates a nil object
		Object();
		virtual ~Object();

		// Create a new, independent copy of this object.
		virtual std::unique_ptr<Object> NewCopy() const;

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
		virtual std::unique_ptr<Object> NewCopy() const;

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
		virtual std::unique_ptr<Object> NewCopy() const;

		// Return the value stored in this object
		double GetValue() const;
	};

	// --------------------------------------------------------------------
	// Class: ObjString
	// Wrapper for a string type
	template <class T, class _Tc>
	class ObjStr  : public Object
	{
	private:
		T str;

	public:
		ObjStr(const T& str) : Object(TYPE_STRING), str(str)
		{
		}

		ObjStr(const ObjStr<T, _Tc>& other) : Object(TYPE_STRING)
		{
			str = other.str;
		}

		// Create a new, independent copy of this object.
		virtual std::unique_ptr<Object> NewCopy() const
		{
			return std::unique_ptr<Object>(new ObjStr<T, _Tc>(*this));
		}

		// Return the value stored in this object
		const _Tc* GetValue() const
		{
			return str.c_str();
		}
	};

	typedef ObjStr<std::string, char> ObjString;
	typedef ObjStr<std::wstring, wchar_t> ObjWString;

	class ObjBlob : public Object
	{
	private:
		std::vector<BYTE> data;
	public:
		ObjBlob(BYTE* data, size_t size);
		ObjBlob(const ObjBlob& other);

		BYTE* GetData(size_t& size);

		// Create a new, independent copy of this object.
		virtual std::unique_ptr<Object> NewCopy() const;
	};

	// --------------------------------------------------------------------
	// Class: ObjTable
	// Wrapper for an associative container
	class ObjTable : public Object
	{
	private:
		typedef std::pair<Object::unique_ptr, Object::unique_ptr> pair_t;
		typedef std::map<Object::unique_ptr, Object::unique_ptr> table_t;
		table_t table;

		// Copies other into this object
		void CopyTable(const ObjTable& other);

		// Frees the table associated with this object
		void FreeTable();

	public:
		ObjTable(const std::map<std::string, std::string>& table);
		ObjTable(const std::map<std::string, std::unique_ptr<Object>>& table);
		ObjTable(const table_t& table);

		~ObjTable();

		ObjTable & operator=(const ObjTable &rhs);
		ObjTable(const ObjTable& other );

		// Create a new, independent copy of this object.
		virtual std::unique_ptr<Object> NewCopy() const;

		// Get value at index
		//const Object* operator [] (size_t i);
		// Returns the value associated with the specified key
		//const Object& operator [] (const Object& key);
	};

	// --------------------------------------------------------------------
	// Memory commands
	BOOL WriteBytes(DWORD dwAddress, LPVOID lpBuffer, DWORD dwCount);
	BOOL ReadBytes(DWORD dwAddress, LPVOID lpBuffer, DWORD dwCount);
	std::vector<DWORD> FindSignatures(LPBYTE sigBuffer, LPBYTE sigWildCard,
		DWORD sigSize, LPBYTE pBuffer, DWORD size);
	bool FindSignature(LPBYTE sigBuffer, LPBYTE sigWildCard, 
		DWORD sigSize, LPBYTE pBuffer, DWORD size, DWORD occurance, DWORD& result);
	BOOL CreateCodeCave(DWORD dwAddress, BYTE cbSize, VOID (*pFunction)());
		
	// --------------------------------------------------------------------
	//
	// Windows error stuff
	// Formats the return value of GetLastError into a string
	void GetLastErrorAsText(std::string& out);
}
