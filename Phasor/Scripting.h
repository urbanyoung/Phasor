#pragma once

#include <map>
#include <vector>
#include <list>

typedef unsigned long DWORD;

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
	// Forward declare the relevant classes for each implementation
	namespace Lua
	{
		class State;
		class Table;
	}

	class Object;
	class Result;

	// --------------------------------------------------------------------
	// Class: Caller
	// Provides an interface for passing parameters to scripts.
	class Caller
	{
	private:
		std::list<Object*> args;

		void SetData(const Caller& other);
		void Free();		

	public:
		Caller();
		Caller(const Caller& other);
		~Caller();
		Caller& operator=(const Caller& rhs);

		// Adds an argument to the list, which is passed to the next function called
		void AddArg(bool b);
		void AddArg(const char* str);
		void AddArg(int value);
		void AddArg(DWORD value);
		void AddArg(float value);
		void AddArg(double value);	
		void AddArg(const std::map<std::string, std::string>& table);	

		// Calls the specified function on all loaded scripts.
		// The vector returned is internal to this class and shouldn't be
		// modified (i couldn't be bothered wrapping it up). It has no
		// guaranteed lifetime and shouldn't be stored.
		Result Call(const char* function);
		Result Call(Lua::State* state, const char* function, bool* found=0, bool erase=true);
	};
	
	// --------------------------------------------------------------------
	// Class: Result
	// Provides an interface for retrieving values from scripts
	class Result
	{
	private:
		std::vector<Object*> result;

		void SetData(const Result& other);
		void Free();

		// Constructs the result and takes ownership of the memory.
		Result(const std::vector<Object*>& result);

		Result();

	public:
		Result(const Result& other);
		Result& operator=(const Result& rhs);
		~Result();

		size_t size() const;

		const Object* operator [] (size_t index);

		friend class Caller;
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

		virtual Object* NewCopy() const;
		Object(obj_type type);

	public:

		Object();
		virtual ~Object();

		obj_type GetType() const;

		friend class ObjTable;
		friend class Caller;
		friend class Result;
	};

	class ObjBool : public Object
	{
	private:
		bool b;
		ObjBool* NewCopy() const;

	public:
		ObjBool(bool b);
		~ObjBool();

		ObjBool & operator=(const ObjBool &rhs); // check for self assignment. see http://courses.cms.caltech.edu/cs11/material/cpp/donnie/cpp-ops.html
		ObjBool(const ObjBool& other );	

		bool GetValue() const;
	};

	class ObjNumber : public Object
	{
	private:
		double value;
		ObjNumber* NewCopy() const;

	public: 
		ObjNumber(int value);
		ObjNumber(DWORD value);
		ObjNumber(double value);
		ObjNumber(float value);
		~ObjNumber();

		ObjNumber & operator=(const ObjNumber &rhs); // check for self assignment. see http://courses.cms.caltech.edu/cs11/material/cpp/donnie/cpp-ops.html
		ObjNumber(const ObjNumber& other ); // maybe move into derived classes

		double GetValue() const;
	};

	class ObjString  : public Object
	{
	private:
		char* str;

		void CopyString(const char* str);
		ObjString* NewCopy() const;

	public:
		ObjString(const char* str);
		~ObjString();

		ObjString & operator=(const ObjString &rhs); // check for self assignment. see http://courses.cms.caltech.edu/cs11/material/cpp/donnie/cpp-ops.html
		ObjString(const ObjString& other ); // maybe move into derived classes

		const char* GetValue() const;
	};

	class ObjTable : public Object
	{
	private:
		std::map<Object*, Object*> table;

		ObjTable* NewCopy() const;
		void CopyTable(const ObjTable& other);
		void FreeTable();

	public:
		ObjTable(const std::map<std::string, std::string>& table);
		ObjTable(Lua::Table* table); // defined in Lua.cpp
		~ObjTable();

		ObjTable & operator=(const ObjTable &rhs); // check for self assignment. see http://courses.cms.caltech.edu/cs11/material/cpp/donnie/cpp-ops.html
		ObjTable(const ObjTable& other ); // maybe move into derived classes

		// Get value at index
		//const Object* operator [] (size_t i);
		// Get value at key
		const Object* operator [] (const Object& key);
	};
}