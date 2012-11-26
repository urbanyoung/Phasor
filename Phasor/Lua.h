#pragma once

#include <list>
#include <string>
#include <deque>
#include <memory>
#include "..\lua\lua.hpp"
#include "Manager.h"

// define object ownerships for Lua stuff. Each object needs to refer to
// the lua state, but atm the lua state is a unique_ptr. 
// Maybe have the objects use a weak_ptr to State and its factory method
// returns a shared_ptr
namespace Lua
{
	typedef Manager::MObject MObject;
	typedef Manager::MObjBool MObjBool;
	typedef Manager::MObjNumber MObjNumber;
	typedef Manager::MObjString MObjString;
	typedef Manager::MObjTable MObjTable;

	class State;
	class Object;
	class Nil;
	class Boolean;
	class Number;
	class String;
	class Table;
	class LuaFunction;
	class CFunction;

	// global ones (use lua references)
	class GBoolean;
	class GNumber;
	class GString;
	class GTable;

	enum Type
	{
		Type_Nil = 0,
		Type_Boolean = 1,
		Type_Number = 3,
		Type_String = 4,
		Type_Table = 5,
		Type_Function = 6
	};	

	//-----------------------------------------------------------------------------------------
	// Class: State
	// This class is analogous to lua_State 
	//

	class State : public Manager::ScriptState
	{
	private:
		// Underlying Lua state
		lua_State* L;	
		std::list<std::unique_ptr<CFunction>> registeredFunctions;
	public:

		// Creates a new state
		State(const char* file);

		// Destroys the state
		~State();

		// Destroys the state
		static void Close(State* state);

		// Loads and runs a file
		void DoFile(const char* filename);

		// Loads and runs a string
		void DoString(const char* str);

		// Gets a global value
		std::unique_ptr<Object> GetGlobal(const char* name);

		// Sets a global value
		void SetGlobal(const char* name, const Object& object);

		// Pop an object off the Lua stack and return it.
		std::unique_ptr<Object> PopLuaObject();
		std::unique_ptr<Object> PeekLuaObject();
		std::unique_ptr<MObject> PopMObject();
		std::unique_ptr<MObject> PeekMObject();

		// Push a generic object onto the Lua stack
		void Push(const MObject& object);

		// The below functions are for creating GLOBAL Lua objects,
		// they can be tracked and thus updated.
		/*std::unique_ptr<GBoolean> GlobalBoolean(const char* name, bool value);
		std::unique_ptr<GNumber> GlobalNumber(const char* name, double value);
		std::unique_ptr<GString> GlobalString(const char* name, const char* value);
		std::unique_ptr<GTable> GlobalTable();*/
		void RegisterFunction(const Manager::ScriptCallback* cb);

		// Checks if the specified function is defined in the script
		bool HasFunction(const char* name);

		// Calls a function with an optional timeout
		MObject::unique_deque Call(const char* name,
			const MObject::unique_list& args, int timeout = 0);
		MObject::unique_deque Call(const char* name, int timeout = 0);

		// Raises an error
		void Error(const char* _Format, ...);

		friend class Object;
		friend class Nil;
		friend class Boolean;
		friend class Number;
		friend class String;
		friend class Table;
		friend class LuaFunction;
		friend class CFunction;
	};

	//
	//-----------------------------------------------------------------------------------------
	// Class: Object
	// Lua value wrapper
	//
	// Objects should not out live the State which created them. The creator
	// is responsible for ensuring this. 
	// These objects are only created by PopStackObject and are simply meant
	// as local CPP wrappers for Lua types.
	class Object
	{	
	private:
		Object(const Object& other);
		
		//Object operator=(const Object& other);

	protected:
		// State the object resides in
		State* state;

		// Type of object stored
		Type type;	

		Object(State* state);
		Object(State* state, Type type);

	public:

		// Push the contained value onto the Lua stack.
		virtual void Push() const = 0;

		// Deletes the object
		virtual ~Object();

		// Returns the object type
		Type GetType() const { return type; };

		// Returns a copy of the object
		/*virtual std::unique_ptr<Object> Copy();

		// Returns a copy of the object in another state
		virtual std::unique_ptr<Object> CopyTo(State* state);

		// Convert this object into a generic one.
		virtual std::unique_ptr<Common::Object> ToGeneric() const;*/
	};

	//
	//-----------------------------------------------------------------------------------------
	// Class: Nil
	// Lua nil wrapper
	class Nil : public Object
	{
	public:
		Nil(State* state);
		void Push() const;

		friend class State;
	};

	//
	//-----------------------------------------------------------------------------------------
	// Class: Boolean
	// Lua boolean wrapper
	//
	class Boolean : public Object
	{
	private:
		bool value;

	public:
		Boolean(State* state, bool value);
		void Push() const;
		bool GetValue();
		void SetValue(bool value);
	};

	//
	//-----------------------------------------------------------------------------------------
	// Class: Number
	// Lua number wrapper
	//

	class Number : public Object
	{
	private:
		double value;

	public:
		Number(State* state, double value);
		void Push() const;
		double GetValue();
		void SetValue(double value);
	};

	//
	//-----------------------------------------------------------------------------------------
	// Class: String
	// Lua string wrapper
	//
	class String : public Object
	{
	private:
		std::string value;

	public:
		String(State* state, const std::string& value);
		void Push() const;
		std::string GetValue();
		void SetValue(const std::string& value);
	};

	//
	//-----------------------------------------------------------------------------------------
	// Class: Table
	// Lua table wrapper
	//
	/*class Table : public Object
	{
	private:
		std::map<Object, Object> table;

	public:
		Table(State* state, const std::map<Object,Object>& table);
		void Push() const;
		
		const Object& operator[](const Object& key);
		const Object& operator[](int key);

		//void SetValue(double value);
	};*/

	/*class Table : public Object
	{
	private:
		// Creates a new table
		Table(State* state);

	public:
		// Gets a value from a key
		std::unique_ptr<Object> GetValue(int key);
		std::unique_ptr<Object> GetValue(const char* key);
		std::unique_ptr<Object> GetValue(Object* key);

		// Sets a key to a value
		void SetValue(int key, Object& value);
		void SetValue(const char* key,  Object& value);
		void SetValue(Object* key, Object& value);

		virtual std::unique_ptr<Object> Copy();

		// Returns a copy of the object
		virtual std::unique_ptr<Object> CopyTo(State* state);

		// Convert this object into a generic one.
		virtual std::unique_ptr<Common::Object> ToGeneric();

		friend class State;
	};*/

	//
	//-----------------------------------------------------------------------------------------
	// Class: Function
	// Lua function wrapper
	// 
	class LuaFunction : public Object
	{
		int ref;
	public:
		LuaFunction(State* state, int ref);
		~LuaFunction();
		void Push() const;

		// Calls the Lua function from C with an optional timeout
		MObject::unique_deque 
			Call(const MObject::unique_list& args, int timeout = 0);
	};

	// --------------------------------------------------------------------
	// Class: CFunction
	// Register a C function that can be called from Lua
	class CFunction : public Object
	{
	private:
		// C function associated with this function
		const Manager::ScriptCallback* cb;

		// Calls the C function from Lua
		static int LuaCall(lua_State* L);

		// Formats a message describing an argument error
		std::string DescribeError(lua_State* L, int narg, int got, int expected);

		// Raises the Lua error, function never returns.
		std::string RaiseError(lua_State* L, int narg, int got, int expected);

	public:
		// Creates a new C function
		CFunction(State* state, const Manager::ScriptCallback* cb);
		void Push() const;

	};
	//
	//-----------------------------------------------------------------------------------------
}
