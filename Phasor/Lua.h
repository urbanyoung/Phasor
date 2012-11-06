#pragma once

#include <list>
#include <string>
#include <deque>
#include <map>
#include "..\lua\lua.hpp"
#include "Manager.h"

namespace Lua
{
	class State;
	class Object;
	class Nil;
	class Boolean;
	class Number;
	class String;
	class Table;
	class Function;

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

		// List of objects in the state
		std::list<Object*> objects;

		// Creates a new state
		State();

		// Destroys the state
		~State();

		// Creates a new object with a value of nil
		Object* NewObject();

	public:
		// Creates a new state
		static State* NewState();

		// Destroys the state
		static void Close(State* state);

		// Loads and runs a file
		void DoFile(const char* filename);

		// Loads and runs a string
		void DoString(const char* str);

		// Gets a global value
		Object* GetGlobal(const char* name);

		// Sets a global value
		void SetGlobal(const char* name, Object* object);

		// Creates a new nil
		Nil* NewNil();

		// Creates a new boolean
		Boolean* NewBoolean(bool value);

		// Creates a new number
		Number* NewNumber(double value);

		// Creates a new string
		String* NewString(const char* value);

		// Creates a new table
		Table* NewTable();

		// Creates a new function
		Function* NewFunction(std::deque<Object*> (*func)(State*, std::deque<Object*>&));

		// Create a new named function
		void RegisterFunction(const char* name, std::deque<Object*> (*func)(State*, std::deque<Object*>&));

		// Checks if the specified function is defined in the script
		bool HasFunction(const char* name);

		// Calls a function with an optional timeout
		// Caller is responsible for memory management of return vector
		std::deque<Manager::MObject*> Call(const char* name, const std::list<Manager::MObject*>& args, int timeout = 0);
		std::deque<Manager::MObject*> Call(const char* name, int timeout = 0);

		// Raises an error
		void Error(const char* _Format, ...);

		Manager::MObject* ToNativeObject(const Common::Object* in);

		friend class Object;
		friend class Nil;
		friend class Boolean;
		friend class Number;
		friend class String;
		friend class Table;
		friend class Function;
	};

	//
	//-----------------------------------------------------------------------------------------
	// Class: Object
	// Lua value wrapper
	//

	class Object : public Manager::MObject
	{
		Type type;

	protected:
		// State the object resides in
		State* state;

		// Lua registry reference; where the value is stored
		int ref;

		// Creates a new object with a value of 0
		Object(State* state);

		// Deletes the object
		~Object();

		// Gets the objects value and pushes it on the stack
		void Push() const;

		// Pops a value off the stack and sets the object
		void Pop();

		// Gets a value off the stack without removing it and sets the object
		void Peek();

	public:
		// Deletes the object
		void Delete();

		// Returns the object type
		Type GetType() const;

		// Returns a copy of the object
		Object* Copy();

		// Returns a copy of the object in another state
		Object* CopyTo(State* state);

		// Convert this object into a generic one.
		Common::Object* ToGeneric() const;

		friend class State;
		friend class Table;
		friend class Function;
	};

	//
	//-----------------------------------------------------------------------------------------
	// Class: Nil
	// Lua nil wrapper
	//

	class Nil : public Object
	{
	private:
		// Creates a new nil
		Nil(State* state);

	public:
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
		// Creates a new boolean
		Boolean(State* state, bool value);

	public:
		// Returns the value of the boolean
		bool GetValue();

		// Sets the value of the boolean
		void SetValue(bool value);

		// Convert this object into a generic one.
		Common::ObjBool* GetGeneric();

		friend class State;
	};

	//
	//-----------------------------------------------------------------------------------------
	// Class: Number
	// Lua number wrapper
	//

	class Number : public Object
	{
	private:
		// Creates a new number
		Number(State* state, double value);

	public:
		// Returns the value of the number
		double GetValue();

		// Sets the value of the number
		void SetValue(double value);

		// Convert this object into a generic one.
		Common::ObjNumber* GetGeneric();

		friend class State;
	};

	//
	//-----------------------------------------------------------------------------------------
	// Class: String
	// Lua string wrapper
	//

	class String : public Object
	{
	private:
		// Creates a new string
		String(State* state, const char* value);

	public:
		// Returns the value of the string
		const char* GetValue();

		// Sets the value of the string
		void SetValue(const char* value);

		// Convert this object into a generic one.
		Common::ObjString* GetGeneric();

		friend class State;
	};

	//
	//-----------------------------------------------------------------------------------------
	// Class: Table
	// Lua table wrapper
	//

	class Table : public Object
	{
	private:
		// Creates a new table
		Table(State* state);

	public:
		// Gets a value from a key
		Object* GetValue(int key);
		Object* GetValue(const char* key);
		Object* GetValue(Object* key);

		// Sets a key to a value
		void SetValue(int key, Object* value);
		void SetValue(const char* key, Object* value);
		void SetValue(Object* key, Object* value);

		// Convert this object into a generic one.
		Common::ObjTable* GetGeneric();

		friend class State;
	};

	//
	//-----------------------------------------------------------------------------------------
	// Class: Function
	// Lua function wrapper
	//

	class Function : public Object
	{
	private:
		std::deque<Object*> (*func)(State*, std::deque<Object*>&);

		// Creates a new C function
		Function(State* state, std::deque<Object*> (*func)(State*, std::deque<Object*>&));

		// Calls the C function from Lua
		static int LuaCall(lua_State* L);

	public:
		// Calls the Lua function from C with an optional timeout
		std::deque<MObject*> Call(const std::list<MObject*>& args, int timeout = 0);

	public:
		friend class State;
	};

	//
	//-----------------------------------------------------------------------------------------
}