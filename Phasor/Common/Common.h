#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <deque>
#include <memory>

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
		TYPE_BLOB,
		TYPE_ANY
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
		Object(const Object& other);

	public:
		typedef std::unique_ptr<Object> unique_ptr;
		typedef std::list<unique_ptr> unique_list;
		typedef std::deque<unique_ptr> unique_deque;

		// Creates a nil object
		Object();
		virtual ~Object();

		//virtual Object& operator=(const Object &rhs); 

		// Create a new, independent copy of this object.
		virtual std::unique_ptr<Object> NewCopy() const;

		// Converts the object to the specified type, if possible.
		// If no conversion is possible false is returned.
		virtual bool ConvertTo(obj_type type, std::unique_ptr<Object>* out) const;
		
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

		// Converts the object to the specified type, if possible.
		// If no conversion is possible false is returned.
		virtual bool ConvertTo(obj_type type, std::unique_ptr<Object>* out) const;

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

		// Converts the object to the specified type, if possible.
		// If no conversion is possible false is returned.
		virtual bool ConvertTo(obj_type type, std::unique_ptr<Object>* out) const;

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

		template <class StrType>
		const bool ToNumber(double& out) const;

		template <>
		const bool ToNumber<std::string>(double& out) const
		{
			char* end;
			double value = strtod(str.c_str(), &end);
			if (end == str.c_str()) return false;
			out = value;
			return true;
		}

		template <>
		const bool ToNumber<std::wstring>(double& out) const
		{
			wchar_t* end;
			double value = wcstod(str.c_str(), &end);
			if (end == str.c_str()) return false;
			out = value;
			return true;
		}

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

		
		// Converts the object to the specified type, if possible.
		// If no conversion is possible false is returned.
		virtual bool ConvertTo(obj_type type, std::unique_ptr<Object>* out) const
		{
			bool success = true;
			switch (type)
			{
			case TYPE_NUMBER:
				{
					double val;
					success = ToNumber<T>(val);
					if (success) out->reset(new ObjNumber(val));
				} break;
			case TYPE_BOOL:
				{
					double val;
					success = ToNumber<T>(val);
					if (success) {
						if (val == 0 || val == 1) out->reset(new ObjBool(val == 1));
						else success = false;
					}
				} break;
			default:
				{
					success = false;
				} break;
			}
			return success;
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
	public:
		typedef std::pair<Object::unique_ptr, Object::unique_ptr> pair_t;
		typedef std::map<Object::unique_ptr, Object::unique_ptr> table_t;
	private:
		table_t table;
		
		// Copies other into this object
		void CopyTable(const ObjTable& other);

		// Frees the table associated with this object
		void FreeTable();

	public:
		ObjTable(const std::map<std::string, std::string>& table);
		ObjTable(const std::map<std::string, std::unique_ptr<Object>>& table);
		ObjTable(const std::vector<std::string>& data, size_t firstkey);
		ObjTable();
		~ObjTable();

		ObjTable & operator=(const ObjTable &rhs);
		ObjTable(const ObjTable& other );

		// Create a new, independent copy of this object.
		virtual std::unique_ptr<Object> NewCopy() const;

		size_t size() const;
		table_t::const_iterator begin() const;
		table_t::const_iterator end() const;
		void insert(pair_t pair);
		// Get value at index
		//const Object* operator [] (size_t i);
		// Returns the value associated with the specified key
		//const Object& operator [] (const Object& key);
	};

	// --------------------------------------------------------------------
	// Memory commands
	BOOL WriteBytes(DWORD dwAddress, const LPVOID lpBuffer, DWORD dwCount);
	BOOL WriteString(DWORD dwAddress, const char* str);
	BOOL ReadBytes(DWORD dwAddress, LPVOID lpBuffer, DWORD dwCount);
	std::vector<DWORD> FindSignatures(LPBYTE sigBuffer, LPBYTE sigWildCard,
		DWORD sigSize, LPBYTE pBuffer, DWORD size);
	bool FindSignature(LPBYTE sigBuffer, LPBYTE sigWildCard, 
		DWORD sigSize, LPBYTE pBuffer, DWORD size, DWORD occurance, DWORD& result);
	BOOL CreateCodeCave(DWORD dwAddress, BYTE cbSize, VOID (*pFunction)());
}
