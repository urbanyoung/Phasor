#include "Lua.h"
#include "Common/Common.h"
#include <sstream>

namespace Lua
{		

	//-----------------------------------------------------------------------------------------
	// Class: State
	// Lua state wrapper
	//

	// Creates a new state
	State::State(const char* file)
	{
		// Create a new Lua state
		this->L = luaL_newstate();

		// Check if an error occured
		if (!this->L)
			throw std::exception("can't open lua state.");

		// Load the Lua libraries into the state
		luaL_openlibs(this->L);

		if (luaL_dofile(this->L, file))
		{
			std::string error = lua_tostring(this->L, -1);
			lua_pop(this->L, 1);
			lua_close(this->L);

			throw std::exception(error.c_str());
		}
	}

	// Destroys the state
	State::~State()
	{
		// Destroy the Lua state
		lua_close(this->L);
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

	std::unique_ptr<MObject> State::PeekMObject()
	{
		std::unique_ptr<MObject> object;
		switch ((Type)lua_type(L, -1))
		{
		case Type_Boolean:
			{
				bool value = lua_toboolean(L, -1) == 1;
				object = std::unique_ptr<MObject>(new MObjBool(value));
			} break;
		case Type_Number:
			{
				double value = lua_tonumber(L, -1);
				object = std::unique_ptr<MObject>(new MObjNumber(value));
			} break;
		case Type_String:
			{
				const char* value = lua_tostring(L, -1);
				object = std::unique_ptr<MObject>(new MObjString(value));
			} break;
		case Type_Table:
			{
				// not ready to support tables yet
				// todo: support tables
				object = std::unique_ptr<MObject>(new MObject());
			} break;

		default: // nil
			{
				object = std::unique_ptr<MObject>(new MObject());			
			} break;
		}
		return object;
	}

	std::unique_ptr<MObject> State::PopMObject()
	{
		std::unique_ptr<MObject> object = PeekMObject();
		lua_pop(L, 1);
		return object;
	}

	std::unique_ptr<Object> State::PeekLuaObject()
	{
		std::unique_ptr<Object> object;

		switch ((Type)lua_type(L, -1))
		{
		case Type_Boolean:
			{
				bool value = lua_toboolean(L, -1) == 1;
				object = std::unique_ptr<Object>(new Boolean(this, value));
			} break;
		case Type_Number:
			{
				double value = lua_tonumber(L, -1);
				object = std::unique_ptr<Object>(new Number(this, value));
			} break;
		case Type_String:
			{
				const char* value = lua_tostring(L, -1);
				object = std::unique_ptr<Object>(new String(this, value));
			} break;
		case Type_Table:
			{
				// not ready to support tables yet
				// todo: support tables
				object = std::unique_ptr<Object>(new Nil(this));
			} break;
		case Type_Function:
			{
				int ref = luaL_ref(L, LUA_REGISTRYINDEX); // this pops
				object = std::unique_ptr<Object>(new LuaFunction(this, ref));
				object->Push();
			} break;

		default: // nil
			{
				object = std::unique_ptr<Object>(new Nil(this));			
			} break;
		}
		return object;
	}

	// Creates a new object with type w/e is on top of stack
	std::unique_ptr<Object> State::PopLuaObject()
	{
		std::unique_ptr<Object> object = PeekLuaObject();
		lua_pop(L, 1);
		return object;
	}

	void State::Push(const MObject& object)
	{
		switch (object.GetType())
		{
		case Common::TYPE_BOOL:
			{
				MObjBool& b = (MObjBool&)object;
				lua_pushboolean(L, b.GetValue());
			} break;
		case Common::TYPE_NUMBER:
			{
				MObjNumber& n = (MObjNumber&)object;
				lua_pushnumber(L, n.GetValue());
			} break;
		case Common::TYPE_STRING:
			{
				MObjString& s = (MObjString&)object;
				lua_pushstring(L, s.GetValue());
			} break;
		case Common::TYPE_TABLE: // not supported yet
		default:
			{
				lua_pushnil(L);
			} break;
		}
	}

	// Gets a global value
	std::unique_ptr<Object> State::GetGlobal(const char* name)
	{
		lua_getglobal(this->L, name);
		return PopLuaObject();
	}

	// Sets a global value
	void State::SetGlobal(const char* name, const Object& object)
	{
		object.Push();
		lua_setglobal(this->L, name);
	}

	// Create a new named function
	void State::RegisterFunction(const Manager::ScriptCallback* cb)
	{
		std::unique_ptr<CFunction> function(new CFunction(this, cb));
		this->SetGlobal(cb->name, *function);
		registeredFunctions.push_back(std::move(function));
	}

	// Checks if the specified Lua function is defined in the script
	bool State::HasFunction(const char* name)
	{
		return GetGlobal(name)->GetType() == Type_Function;
	}

	// Calls a Lua function from C
	MObject::unique_deque State::Call(const char* name,
		const MObject::unique_list& args, int timeout)
	{
		std::unique_ptr<Object> _function = this->GetGlobal(name);
		LuaFunction& function = (LuaFunction&)*_function;
		if (function.GetType() != Type_Function)
			throw std::exception("lua: Attempted function call on non-function entity.");

		return function.Call(args, timeout);
	}

	// Calls a function
	// Caller is responsible for memory management of return vector
	MObject::unique_deque State::Call(const char* name, int timeout)
	{
		const MObject::unique_list args;
		return this->Call(name, args, timeout);
	}

	// Raises an error
	/*void State::Error(const char* _Format, ...)
	{
		// not exception safe
		// will probably change to either
		// http://www.codeproject.com/Articles/15115/How-to-Format-a-String
		// or http://www.boost.org/doc/libs/1_42_0/libs/format/doc/format.html
		va_list _Args;
		va_start(_Args, _Format);

		int count = _vscprintf(_Format, _Args);

		char* msg = new char[count + 1];
		memset(msg, 0, count + 1);
		vsprintf(msg, _Format, _Args);

		std::string error = msg;

		delete[] msg;

		luaL_error(this->L, error.c_str());
	}*/

	Object::Object(State* state) : type(Type_Nil)
	{
		this->state = state;
	}

	Object::Object(State* state, Type type) : state(state), type(type){}

	// Deletes the object
	Object::~Object()
	{
	}

	Nil::Nil(State* state) : Object(state, Type_Nil)
	{
	}

	void Nil::Push() const
	{
		lua_pushnil(state->L);
	}

	//
	//-----------------------------------------------------------------------------------------
	// Class: Boolean
	// Lua boolean wrapper
	//
	Boolean::Boolean(State* state, bool value) : Object(state, Type_Boolean)
	{
		SetValue(value);
	}

	// Returns the value of the boolean
	bool Boolean::GetValue()
	{
		return value;
	}

	// Sets the value of the boolean
	void Boolean::SetValue(bool value)
	{
		this->value = value;
	}

	void Boolean::Push() const
	{
		lua_pushboolean(state->L, value);
	}

	//
	//-----------------------------------------------------------------------------------------
	// Class: Number
	// Lua number wrapper
	//
	Number::Number(State* state, double value) : Object(state, Type_Number)
	{
		SetValue(value);
	}	

	// Returns the value of the number
	double Number::GetValue()
	{
		return value;
	}

	// Sets the value of the number
	void Number::SetValue(double value)
	{
		this->value = value;
	}

	void Number::Push() const 
	{
		lua_pushnumber(state->L, value);
	}
	//
	//-----------------------------------------------------------------------------------------
	// Class: String
	// Lua string wrapper
	//
	String::String(State* state, const std::string& value) : Object(state, Type_String)
	{
		SetValue(value);
	}

	// Returns the value of the string
	std::string String::GetValue()
	{
		return value;
	}

	// Sets the value of the string
	void String::SetValue(const std::string& value)
	{
		this->value = value;
	}

	void String::Push() const 
	{
		lua_pushstring(state->L, value.c_str());
	}
	
	// -------------------------------------------------------------------
	CFunction::CFunction(State* state, const Manager::ScriptCallback* cb)
		: Object(state), cb(cb)
	{
		//printf("Creating named function : %s\n", this->cb->name);
	}

	void CFunction::Push() const
	{
		lua_pushlightuserdata(this->state->L, (void*)this);
		lua_pushcclosure(this->state->L, CFunction::LuaCall, 1);
	}

	// Formats a message describing an argument error
	std::string CFunction::DescribeError(lua_State* L, int narg, int got, int expected)
	{
		std::stringstream ss;
		ss << "bad argument #" << narg << " to '" << this->cb->name << "' ("
			<< lua_typename(L, expected) << " expected got " << lua_typename(L, got) << ")";
		return ss.str();
	}

	int CFunction::RaiseError(lua_State* L, int narg, int got, int expected)
	{
		// this function never returns (throws exc
		return luaL_error(L, DescribeError(L, narg, got, expected).c_str());
	}

	// Calls the C function from Lua
	int CFunction::LuaCall(lua_State* L)
	{
		// Get the CFunction class from upvalue
		CFunction* function = (CFunction*)lua_touserdata(L, lua_upvalueindex(1));	

		const int maxargs_all = function->cb->fmt.size(); // max any function can receive
		int maxargs = 0; // max number of arguments this specific function expects
		int minargs = function->cb->minargs; // minimum allowed
		int nargs = lua_gettop(L); // number of args received

		// Make sure the received arguments are within the extreme limits
		if (nargs < minargs) { 
			std::stringstream ss;
			ss << "'" << function->cb->name << "' expects at least " << minargs  
				<< " argument(s) and received " << nargs;
			return luaL_error(L, ss.str().c_str()); // noret
		} else if (nargs > maxargs_all) {
			std::stringstream ss;
			ss << "'" << function->cb->name << "' expects at most " << maxargs_all  
				<< " argument(s) and received " << nargs;
			return luaL_error(L, ss.str().c_str()); // noret
		}

		MObject::unique_deque args;		

		// Check the arguments are of expected type and add them to args.
		for (int i = 0; i < nargs; i++)
		{
			if (function->cb->fmt[i] == Common::TYPE_NIL)
				break; // no more args expected.

			int indx = i + 1;
			maxargs++;

			std::unique_ptr<MObject> obj;
			switch (function->cb->fmt[i])
			{
			case Common::TYPE_BOOL:
				{
					int b;
					// phasor_legacy: this "fix" is specific to Phaosr
					// old versions accepted 0 as boolean false but Lua treats
					// it as boolean true.
					if (lua_type(L, indx) == Type_Number && lua_tonumber(L, indx) == 0) 
						b = 0;
					else
						b = lua_toboolean(L, indx);
					obj.reset(new MObjBool(b != 0));

					break;
				}
			case Common::TYPE_NUMBER:
				{
					// if it can't be converted luaL_checknumber raises an error
					lua_Number n = luaL_checknumber(L, indx);
					obj.reset(new MObjNumber(n));

					break;
				}
			case Common::TYPE_STRING:
				{
					// checkstring converts stack value and raises an error
					// if no conversion exists
					luaL_checkstring(L, indx);
					obj.reset(new MObjString(lua_tostring(L, indx)));
					break;
				}
			case Common::TYPE_TABLE:
				{
					obj.reset(new MObject());
				/*	int type = lua_type(L, indx);

					if (type != Type_Table) {
						return function->RaiseError(L, i + 1, type, Type_Table);
						// RaiseError never returns
					}*/

					break;
				}
			}
			args.push_front(std::move(obj));
			//lua_pop(L, 1);
		}
		lua_pop(L, nargs);

		// Too many arguments were received
		if (maxargs < nargs) {
			return function->RaiseError(L, maxargs + 1, lua_type(L, maxargs + 1), Type_Nil);
			// RaiseError never returns
		}
		
		// Call the C function
		MObject::unique_list results = 
			Manager::InvokeCFunction(*(function->state), args, function->cb);

		// Push the results on the stack
		for (auto itr = results.begin(); itr != results.end(); ++itr)
			function->state->Push(*(*itr));

		int nresults = results.size();

		return nresults;
	}

	// --------------------------------------------------------------------

	LuaFunction::LuaFunction(State* state, int ref) : Object(state, Type_Function),
		ref(ref)
	{
	}

	LuaFunction::~LuaFunction()
	{
		luaL_unref(state->L, LUA_REGISTRYINDEX, ref);
	}

	void LuaFunction::Push() const
	{
		lua_rawgeti(this->state->L, LUA_REGISTRYINDEX, this->ref);
	}

	// Calls the Lua function from C
	MObject::unique_deque LuaFunction::Call(
		const MObject::unique_list& args, int timeout)
	{
		// Push the function on the stack
		this->Push();

		// Push the arguments on the stack
		for (auto itr = args.begin(); itr != args.end(); ++itr)
			state->Push(**itr);

		// Call the Lua function
		// Check if error occurred
		if (lua_pcall_t(this->state->L, args.size(), LUA_MULTRET, 0, timeout))
		{
			std::string error = lua_tostring(this->state->L, -1);
			lua_pop(this->state->L, -1);

			throw std::exception(error.c_str());
		}

		MObject::unique_deque results;

		// Pop the results off the stack
		int n = lua_gettop(state->L);
		for (int i = 0; i < n; i++)
			results.push_front(state->PopMObject());

		return results;
	}

	//
	//-----------------------------------------------------------------------------------------
}