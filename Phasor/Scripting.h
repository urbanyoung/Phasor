#pragma once

#include "Lua.h"
#include <map>

typedef unsigned int uint_32;

/*
 * Each script is uniquely identified by its name relative to the 
 * scripts folder. So like
 * Scripts/
 *   MyScript/
 *     dostuff.lua
 *     
 *   hello.lua
 *   
 *   dostuff.lua's unique would be MyScript/dostuff
 *   hello.lua's unique id would be hello
 */

// todo: decide what to do when a script function errors (should it be blocked)
namespace Scripting
{
	// --------------------------------------------------------------------
	// Class: Object
	// Provides an interface between Lua and Phasor objects. The derived
	// classes provide specific types.
	// 
	enum obj_type 
	{
		TYPE_NIL = 0, // undefined type
		TYPE_BOOL,
		TYPE_NUMBER,
		TYPE_STRING,
		TYPE_TABLE
	};
	class Object
	{
	private: 
		obj_type type;

	protected:
		Object(obj_type type);

	public:

		static Object* ConvertObject(Lua::Object* obj);

		virtual ~Object();
		obj_type GetType();
		
	};

	class ObjBool : public Object
	{
	private:
		bool b;
	public:
		ObjBool(bool b);
		ObjBool(Lua::Boolean* b);
		~ObjBool();

		ObjBool & operator=(const ObjBool &rhs); // check for self assignment. see http://courses.cms.caltech.edu/cs11/material/cpp/donnie/cpp-ops.html
		ObjBool(const ObjBool& other );
	};

	class ObjNumber : public Object
	{
	private:
		double value;
	public: 
		ObjNumber(int value);
		ObjNumber(uint_32 value);
		ObjNumber(double value);
		ObjNumber(float value);
		ObjNumber(Lua::Number* value);
		~ObjNumber();

		ObjNumber & operator=(const ObjNumber &rhs); // check for self assignment. see http://courses.cms.caltech.edu/cs11/material/cpp/donnie/cpp-ops.html
		ObjNumber(const ObjNumber& other ); // maybe move into derived classes
	};

	class ObjString  : public Object
	{
	private:
		char* str;

		void CopyString(const char* str);

	public:
		ObjString(const char* str);
		ObjString(Lua::String* str);
		~ObjString();

		ObjString & operator=(const ObjString &rhs); // check for self assignment. see http://courses.cms.caltech.edu/cs11/material/cpp/donnie/cpp-ops.html
		ObjString(const ObjString& other ); // maybe move into derived classes
	};

	class ObjTable : public Object
	{
		std::map<Object*, Object*> table;

		Object* ConvertObject(Lua::Object* obj);

	public:
		ObjTable(const std::map<std::string, std::string>& table);
		ObjTable(Lua::Table* table);
		~ObjTable();

		ObjTable & operator=(const ObjTable &rhs); // check for self assignment. see http://courses.cms.caltech.edu/cs11/material/cpp/donnie/cpp-ops.html
		ObjTable(const ObjTable& other ); // maybe move into derived classes

		// Get value at index
		const Object* operator [] (size_t i);
		// Get value at key
		const Object* operator [] (const Object& key);

	};

	// --------------------------------------------------------------------
	// 
	
	// Sets the path to be used by this namespace (where scripts are).
	void SetPath(const char* scriptPath);

	/* Opens the script specified, relative to the scripts directory (and no
	 * extension).
	 * May throw an exception <todo: add specific info>*/
	void OpenScript(const char* script);

	// Closes the specified script, if it exists.
	// Guarantees the script is closed (if it exsists) but may still throw
	// exception if an error occurs in OnScriptUnload
	void CloseScript(const char* script);

	// Calls the specified function on all loaded scripts.
	// Return value is that of a single script, the "acting" return value.
	std::vector<Object*> Call(const char* function, const std::vector<Object*>& args);
	std::vector<Object*> Call(const char* function);

}