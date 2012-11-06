#include "Lua.h"
#include "Common.h"
#include <sstream>

namespace Lua
{		
	typedef Manager::MObject MObject;

	//-----------------------------------------------------------------------------------------
	// Class: State
	// Lua state wrapper
	//

	// Creates a new state
	State::State()
	{
		// Create a new Lua state
		this->L = luaL_newstate();

		// Check if an error occured
		if (!this->L)
			throw std::exception();

		// Load the Lua librarys into the state
		luaL_openlibs(this->L);
	}

	// Destroys the state
	State::~State()
	{
		std::list<Object*>::iterator objects_itr = this->objects.begin();

		// Clean up objects
		while (objects_itr != this->objects.end())
		{
			delete *objects_itr;
			objects_itr = this->objects.erase(objects_itr);
		}

		// Destroy the Lua state
		lua_close(this->L);
	}

	std::vector<Object*> test_func(State* state, std::vector<Object*>& args)
	{
		String* str = (String*)args[0];
		printf("test_func called with %s\n", str->GetValue());
		String* outStr = state->NewString("value returned");
		std::vector<Object*> results;
		results.push_back(outStr);
		return results;
	}

	// Creates a new state
	State* State::NewState()
	{
		State* state = new State();
		state->RegisterFunction("test_func", test_func);

		return state;
	}

	// Destroys the state
	void State::Close(State* state)
	{
		delete state;
	}

	// Loads and runs a file
	void State::DoFile(const char* filename)
	{
		if (luaL_dofile(this->L, filename))
		{
			std::string error = lua_tostring(this->L, -1);
			lua_pop(this->L, 1);

			throw std::exception(error.c_str());
		}
	}

	// Loads and runs a string
	void State::DoString(const char* str)
	{
		if (luaL_dostring(this->L, str))
		{
			std::string error = lua_tostring(this->L, -1);
			lua_pop(this->L, 1);

			throw std::exception(error.c_str());
		}
	}

	// Creates a new object with a value of nil
	Object* State::NewObject()
	{
		Object* object = new Object(this);
		this->objects.push_back(object);

		return object;
	}

	// Gets a global value
	Object* State::GetGlobal(const char* name)
	{
		lua_getglobal(this->L, name);

		Object* object = this->NewObject();
		object->Pop();

		return object;
	}

	// Sets a global value
	void State::SetGlobal(const char* name, Object* object)
	{
		object->Push();
		lua_setglobal(this->L, name);
	}

	// Creates a new nil
	Nil* State::NewNil()
	{
		Nil* object = new Nil(this);
		this->objects.push_back(object);

		return object;
	}

	// Creates a new boolean
	Boolean* State::NewBoolean(bool value)
	{
		Boolean* object = new Boolean(this, value);
		this->objects.push_back(object);

		return object;
	}

	// Creates a new number
	Number* State::NewNumber(double value)
	{
		Number* object = new Number(this, value);
		this->objects.push_back(object);

		return object;
	}

	// Creates a new string
	String* State::NewString(const char* value)
	{
		String* object = new String(this, value);
		this->objects.push_back(object);

		return object;
	}

	// Creates a new table
	Table* State::NewTable()
	{
		Table* object = new Table(this);
		this->objects.push_back(object);

		return object;
	}

	// Creates a new function
	Function* State::NewFunction(std::vector<Object*> (*func)(State*, std::vector<Object*>&))
	{
		Function* object = new Function(this, func);
		this->objects.push_back(object);
			
		return object;
	}

	// Create a new named function
	void State::RegisterFunction(const char* name, std::vector<Object*> (*func)(State*, std::vector<Object*>&))
	{
		Function* function = NewFunction(func);
		this->SetGlobal(name, function);
	}

	// Checks if the specified function is defined in the script
	bool State::HasFunction(const char* name)
	{
		Function* function = (Function*)this->GetGlobal(name);
		bool exists = function->GetType() == Type_Function;
		function->Delete();
		return exists;
	}

	// Calls a function
	// Caller is responsible for memory management of return vector
	std::vector<MObject*> State::Call(const char* name, 
		const std::list<MObject*>& args, int timeout)
	{
		Function* function = (Function*)this->GetGlobal(name);
		if (function->GetType() != Type_Function)
		{
			throw std::exception("lua: Attempted function call on non-function entity.");
		}

		std::vector<MObject*> results = function->Call(args, timeout);
		function->Delete();

		return results;
	}

	// Calls a function
	// Caller is responsible for memory management of return vector
	std::vector<MObject*> State::Call(const char* name, int timeout)
	{
		const std::list<MObject*> args;
		std::vector<MObject*> results = this->Call(name, args, timeout);
		return results;
	}

	// Raises an error
	void State::Error(const char* _Format, ...)
	{
		va_list _Args;
		va_start(_Args, _Format);

		int count = _vscprintf(_Format, _Args);

		char* msg = new char[count + 1];
		memset(msg, 0, count + 1);
		vsprintf(msg, _Format, _Args);

		std::string error = msg;

		delete[] msg;

		luaL_error(this->L, error.c_str());
	}

	MObject* State::ToNativeObject(const Common::Object* in)
	{
		Lua::Object* out = NULL;
		switch (in->GetType())
		{
			case Common::TYPE_NIL: {
				out = NewNil();
			} break;
			case Common::TYPE_BOOL: {
				Common::ObjBool* b = (Common::ObjBool*)in;
				out = NewBoolean(b->GetValue());
			} break;
			case Common::TYPE_NUMBER: {
				Common::ObjNumber* n = (Common::ObjNumber*)in;
				out = NewNumber(n->GetValue());
			} break;
			case Common::TYPE_STRING: {
				Common::ObjString* str = (Common::ObjString*)in;
				out = NewString(str->GetValue());
			} break;
			case Common::TYPE_TABLE: {
				Common::ObjTable* t = (Common::ObjTable*)in;
				Lua::Table* table = NewTable();

				// iterate through table, adding values

				out = table;
			} break;
		}
		return out;
	}

	//
	//-----------------------------------------------------------------------------------------
	// Class: Object
	// Lua value wrapper
	//

	// Creates a new object with a value of nil
	Object::Object(State* state) : Manager::MObject(), type(Type_Nil)
	{
		this->state = state;

		// Push 0 onto stack, can't use nil as it cannot be referenced.
		// This object however, is treated as a nil value until a new
		// value is popped into it.
		lua_pushnumber(this->state->L, 0);

		// Create new reference
		this->ref = luaL_ref(state->L, LUA_REGISTRYINDEX);
	}

	// Deletes the object
	Object::~Object()
	{
		// Destroy reference
		luaL_unref(this->state->L, LUA_REGISTRYINDEX, this->ref);
	}

	// Gets the objects value and pushes it on the stack
	void Object::Push() const
	{
		// Gets the value at the reference
		// Pushes the value to the stack
		lua_rawgeti(this->state->L, LUA_REGISTRYINDEX, this->ref);
	}

	// Pops a value off the stack and sets the object
	void Object::Pop()
	{
		this->type = (Type)lua_type(this->state->L, -1);
		// Pops a value from the stack
		// Sets the reference to the value
		lua_rawseti(this->state->L, LUA_REGISTRYINDEX, this->ref);
	}

	// Gets a value off the stack without removing it and sets the object
	void Object::Peek()
	{
		this->Pop();
		this->Push();
	}

	// Deletes the object
	void Object::Delete()
	{
		for (std::list<Object*>::iterator itr = this->state->objects.begin();
			itr != this->state->objects.end(); ++itr)
		{
			if (*itr == this)
			{
				this->state->objects.erase(itr);

				break;
			}
		}

		delete this;
	}

	// Returns the object type
	Type Object::GetType() const
	{
		return type;
	}

	// Returns a copy of the object
	Object* Object::Copy()
	{
		Object* object = this->state->NewObject();
		this->Push();
		object->Pop();

		return object;
	}

	// Returns a copy of the object in another state
	Object* Object::CopyTo(State* state)
	{
		Object* object = state->NewObject();

		// Push the old value on the stack
		this->Push();

		switch (this->GetType())
		{
		case Type_Boolean:
			{
				int value = lua_toboolean(this->state->L, -1);
				lua_pushboolean(object->state->L, value);

				break;
			}

		case Type_Number:
			{
				double value = lua_tonumber(this->state->L, -1);
				lua_pushnumber(object->state->L, value);

				break;
			}

		case Type_String:
			{
				const char* value = lua_tostring(this->state->L, -1);
				lua_pushstring(object->state->L, value);

				break;
			}

		case Type_Table:
			{
				lua_newtable(object->state->L);

				// Push first key
				lua_pushnil(this->state->L);

				// Add each key value pair to the table
				// lua_next pops a key from the stack
				// Then pushes the key value pair
				while (lua_next(this->state->L, -2))
				{
					Object* oldValue = this->state->NewObject();
					oldValue->Pop();

					Object* oldKey = this->state->NewObject();
					oldKey->Peek(); // Keep key on the stack

					Object* newKey = oldKey->CopyTo(object->state);
					newKey->Push(); // Push key on stack

					Object* newValue = oldKey->CopyTo(object->state);
					newValue->Push(); // Push value on stack

					// Only add if key and value not nil
					if (newKey->GetType() != Type_Nil && newValue->GetType() != Type_Nil)
					{
						// Add key value pair to table
						// Pops key value pair from stack
						lua_rawset(object->state->L, -3);
					}

					// Clean up objects
					oldKey->Delete();
					oldValue->Delete();
					newKey->Delete();
					newValue->Delete();
				}

				break;
			}
		}

		// Pop the old value off the stack
		this->Pop();

		// Pop the new value off the stack
		object->Pop();

		return object;
	}

	Common::Object* Object::ToGeneric() const
	{
		Common::Object* out = NULL;
		switch (GetType())
		{		
		case Lua::Type_Boolean:
			{
				Lua::Boolean* b = (Lua::Boolean*)this;
				out = b->GetGeneric();
			} break;
		case Lua::Type_Number:
			{
				Lua::Number* n = (Lua::Number*)this;
				out = n->GetGeneric();
			} break;
		case Lua::Type_String:
			{
				Lua::String* str = (Lua::String*)this;
				out = str->GetGeneric();
			} break;
		case Lua::Type_Table:
			{
				Lua::Table* table = (Lua::Table*)this;
				out = table->GetGeneric();
			} break;
		default:
			{
				out = new Common::Object();
			} break;
		}
		return out;
	}

	//
	//-----------------------------------------------------------------------------------------
	// Class: Nil
	// Lua nil wrapper
	//

	Nil::Nil(State* state) : Object(state)
	{
		// Object constructor sets default value to nil
	}

	//
	//-----------------------------------------------------------------------------------------
	// Class: Boolean
	// Lua boolean wrapper
	//

	// Creates a new boolean
	Boolean::Boolean(State* state, bool value) : Object(state)
	{
		lua_pushboolean(this->state->L, value);
		this->Pop();
	}

	// Returns the value of the boolean
	bool Boolean::GetValue()
	{
		this->Push();
		bool value = (bool)lua_toboolean(this->state->L, -1);
		this->Pop();

		return value;
	}

	// Sets the value of the boolean
	void Boolean::SetValue(bool value)
	{
		lua_pushboolean(this->state->L, value);
		this->Pop();
	}

	Common::ObjBool* Boolean::GetGeneric()
	{
		return new Common::ObjBool(this->GetValue());
	}

	//
	//-----------------------------------------------------------------------------------------
	// Class: Number
	// Lua number wrapper
	//

	// Creates a new number
	Number::Number(State* state, double value) : Object(state)
	{
		lua_pushnumber(this->state->L, value);
		this->Pop();
	}

	// Returns the value of the number
	double Number::GetValue()
	{
		this->Push();
		double value = lua_tonumber(this->state->L, -1);
		this->Pop();

		return value;
	}

	// Sets the value of the number
	void Number::SetValue(double value)
	{
		lua_pushnumber(this->state->L, value);
		this->Pop();
	}

	Common::ObjNumber* Number::GetGeneric()
	{
		return new Common::ObjNumber(this->GetValue());
	}

	//
	//-----------------------------------------------------------------------------------------
	// Class: String
	// Lua string wrapper
	//

	// Creates a new string
	String::String(State* state, const char* value) : Object(state)
	{
		lua_pushstring(this->state->L, value);
		this->Pop();
	}

	// Returns the value of the string
	const char* String::GetValue()
	{
		this->Push();
		const char* value = lua_tostring(this->state->L, -1);
		this->Pop();

		return value;
	}

	// Sets the value of the string
	void String::SetValue(const char* value)
	{
		lua_pushstring(this->state->L, value);
		this->Pop();
	}

	Common::ObjString* String::GetGeneric()
	{
		return new Common::ObjString(this->GetValue());
	}

	//
	//-----------------------------------------------------------------------------------------
	// Class: Table
	// Lua table wrapper
	//

	// Creates a new table
	Table::Table(State* state) : Object(state)
	{
		lua_newtable(this->state->L);
		this->Pop();
	}

	// Gets a value from a key
	Object* Table::GetValue(int key)
	{
		Object* object = this->state->NewObject();

		this->Push();
		lua_pushnumber(this->state->L, key);
		lua_gettable(this->state->L, -2);
		object->Pop();
		this->Pop();

		return object;
	}

	// Gets a value from a key
	Object* Table::GetValue(const char* key)
	{
		Object* object = this->state->NewObject();

		this->Push();
		lua_pushstring(this->state->L, key);
		lua_gettable(this->state->L, -2);
		object->Pop();
		this->Pop();

		return object;
	}

	// Gets a value from a key
	Object* Table::GetValue(Object* key)
	{
		Object* object = this->state->NewObject();

		this->Push();
		key->Push();
		lua_gettable(this->state->L, -2);
		object->Pop();
		this->Pop();

		return object;
	}

	// Sets a key to a value
	void Table::SetValue(int key, Object* value)
	{
		this->Push();
		lua_pushnumber(this->state->L, key);
		value->Push();
		lua_settable(this->state->L, -3);
		this->Pop();
	}

	// Sets a key to a value
	void Table::SetValue(const char* key, Object* value)
	{
		this->Push();
		lua_pushstring(this->state->L, key);
		value->Push();
		lua_settable(this->state->L, -3);
		this->Pop();
	}

	// Sets a key to a value
	void Table::SetValue(Object* key, Object* value)
	{
		this->Push();
		key->Push();
		value->Push();
		lua_settable(this->state->L, -3);
		this->Pop();
	}

	Common::ObjTable* Table::GetGeneric()
	{
		std::map<Common::Object*, Common::Object*> table;
		this->Push();

		// Push first key
		lua_pushnil(this->state->L);

		// Add each key value pair to the table
		// lua_next pops a key from the stack
		// Then pushes the key value pair
		while (lua_next(this->state->L, -2))
		{
			Object* value = this->state->NewObject();
			value->Pop();

			Object* key = this->state->NewObject();
			key->Peek(); // Keep key on the stack

			// Only add if key and value not nil
			if (key->GetType() != Type_Nil && value->GetType() != Type_Nil)
			{
				table.insert(std::pair<Common::Object*, Common::Object*>
					(key->ToGeneric(), value->ToGeneric()));
			}
		}

		return new Common::ObjTable(table);
	}

	//
	//-----------------------------------------------------------------------------------------
	// Class: Function
	// Lua function wrapper
	//

	Function::Function(State* state, std::vector<Object*> (*func)(State*, std::vector<Object*>&)) : Object(state)
	{
		this->func = func;

		lua_pushlightuserdata(this->state->L, this);
		lua_pushcclosure(this->state->L, Function::LuaCall, 1);
		this->Pop();
	}

	// Calls the C function from Lua
	int Function::LuaCall(lua_State* L)
	{
		// todo: change to use deque 
		// Get the Function class from upvalue
		Function* function = (Function*)lua_touserdata(L, lua_upvalueindex(1));

		std::vector<Object*> args;

		// Pop items off stack (first param is last on stack)
		/*int nparams = lua_gettop(L);
		if (nparams > 0)
		{
			args.reserve(nparams);
			for (int i = nparams - 1; i >= 0; i--)
			{
				Object* object = function->state->NewObject();
				object->Pop();
				args.assign(i,object);
			}
		}*/

		// Pop the arguments off the stack
		while (lua_gettop(L))
		{
			printf("Type pre-check: %i\n", lua_type(L, 1));
			luaL_checkstring(L, 1);
			printf("Type post-check: %i\n", lua_type(L, 1));
			Object* object = function->state->NewObject();
			object->Pop();
			printf("Type %i\n", object->GetType());

			// Add argument to front
			if (args.size())
			{
				std::vector<Object*>::iterator itr = args.begin();
				args.insert(itr, object);
			}
			else
				args.push_back(object);
		}
		
		// Call the C function
		std::vector<Object*> results = function->func(function->state, args);

		// Push the results on the stack
		for (std::vector<Object*>::iterator itr = results.begin(); itr != results.end(); ++itr)
			(*itr)->Push();

		int nresults = results.size();

		std::vector<Object*>::iterator itr = args.begin();

		// Clean up arguments
		while (itr != args.end())
		{
			(*itr)->Delete();
			itr = args.erase(itr);
		}

		itr = results.begin();

		// Clean up results
		while (itr != results.end())
		{
			(*itr)->Delete();
			itr = results.erase(itr);
		}

		return nresults;
	}

	// Calls the Lua function from C
	std::vector<MObject*> Function::Call(const std::list<MObject*>& args, int timeout)
	{
		// todo: change to use deque 
		// Push the function on the stack
		this->Push();

		// Push the arguments on the stack
		for (std::list<MObject*>::const_iterator itr = args.begin(); itr != args.end(); ++itr)
		{
			Object* obj = (Object*)*itr;
			obj->Push();
		}

		// Call the Lua function
		// Check if error occurred
		if (lua_pcall_t(this->state->L, args.size(), LUA_MULTRET, 0, timeout))
		{
			std::string error = lua_tostring(this->state->L, -1);
			lua_pop(this->state->L, -1);

			throw std::exception(error.c_str());
		}

		std::vector<MObject*> results;

		/*int nparams = lua_gettop(this->state->L);
		if (nparams > 0)
		{
			printf("%i results\n", nparams);
			results.reserve(nparams);
			printf("%i vector slots", results.size());
			for (int i = nparams - 1; i >= 0; i--)
			{
				Object* object = this->state->NewObject();
				object->Pop();
				results[i] = object;
			}
		}*/

		// Pop the results off the stack
		while (lua_gettop(this->state->L))
		{
			Object* object = this->state->NewObject();
			object->Pop();

			// Add result to front
			if (results.size())
			{
				std::vector<MObject*>::iterator itr = results.begin();
				results.insert(itr, object);
			}
			else
				results.push_back(object);
		}

		return results;
	}

	//
	//-----------------------------------------------------------------------------------------
}