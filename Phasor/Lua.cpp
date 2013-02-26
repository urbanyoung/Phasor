#include "Lua.h"
#include "Common/Common.h"
#include <sstream>
#include <assert.h>

namespace Lua
{		
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
			throw std::exception("can't create new lua state.");

		// Load the Lua libraries into the state
		luaL_openlibs(this->L);
	}

	// Destroys the state
	State::~State()
	{
		// Destroy the Lua state
		lua_close(this->L);
	}

	void State::DoFile(const char* file)
	{
		if (luaL_dofile(this->L, file))
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

	std::unique_ptr<MObjTable> State::peek_table(int indx)
	{
		std::unique_ptr<MObjTable> table(new MObjTable());
		lua_pushnil(L); // so we get first key		
		while(lua_next(L, indx - 1)) { 
			auto value = pop();
			auto key = peek();

			table->insert(std::pair<MObject::unique_ptr, MObject::unique_ptr>
				(std::move(key->release_object()), std::move(value->release_object())));				
		}
		return table;
	}

	std::unique_ptr<LuaObject> State::peek(int indx)
	{
		std::unique_ptr<LuaObject> object;
		LuaType type = (LuaType)lua_type(L, indx);
		switch (type)
		{
		case Type_Boolean:
			{
				bool value = lua_toboolean(L, indx) == 1;
				object.reset(new LuaObject(value));
			} break;
		case Type_Number:
			{
				double value = lua_tonumber(L, indx);
				object.reset(new LuaObject(value));
			} break;
		case Type_String:
			{
				const char* value = lua_tostring(L, indx);
				object.reset(new LuaObject(value));
			} break;
		case Type_Table:
			{			
				object.reset(new LuaObject(peek_table(indx)));

			} break;
		case Type_Nil:
			{
				object.reset(new LuaObject());	
			} break;
		default:
			{
				lua_pushvalue(L, indx); // luaL_ref pops so copy it
				int ref = luaL_ref(L, LUA_REGISTRYINDEX); // this pops

				// can't handle the value so store its ref
				object.reset(new UnhandledObject(*this, ref, type));
			} break;
		}
		return std::move(object);
	}

	std::unique_ptr<LuaObject> State::pop()
	{
		std::unique_ptr<LuaObject> object = peek();
		lua_pop(L, 1);
		return object;
	}

	void State::push(const MObject& object)
	{
		switch (object.GetType())
		{
		case Common::TYPE_BOOL:
			{
				const MObjBool& b = static_cast<const MObjBool&>(object);
				lua_pushboolean(L, b.GetValue());
			} break;
		case Common::TYPE_NUMBER:
			{
				const MObjNumber& n = static_cast<const MObjNumber&>(object);
				lua_pushnumber(L, n.GetValue());
			} break;
		case Common::TYPE_STRING:
			{
				const MObjString& s = static_cast<const MObjString&>(object);
				lua_pushstring(L, s.GetValue());
			} break;
		
		case Common::TYPE_TABLE:
			{
				const MObjTable& table = static_cast<const MObjTable&>(object);
				lua_createtable(L, table.size(), table.size());

				for (auto itr = table.begin(); itr != table.end(); ++itr) {
					push(*itr->first.get());//key
					push(*itr->second.get());//value
					lua_settable(L, -3);
				}
				
			} break;
		case Common::TYPE_NIL:
		default:
			{
				lua_pushnil(L);
			} break;
		}
	}

	// Create a new named function
	void State::RegisterFunction(const Manager::ScriptCallback* cb)
	{
		std::unique_ptr<CFunction> function(new CFunction(*this, cb));
		set_global(cb->name, *function);
		registeredFunctions.push_back(std::move(function));
	}

	void State::set_global(const char* name, LuaObject& obj)
	{
		obj.push(*this);
		lua_setglobal(L, name);
	}

	std::unique_ptr<LuaObject> State::get_global(const char* name)
	{
		lua_getglobal(L, name);
		return pop();
	}

	// Checks if the specified Lua function is defined in the script
	bool State::HasFunction(const char* name)
	{
		return get_global(name)->get_type() == Type_Function;
	}

	// Calls a Lua function from C
	MObject::unique_deque State::Call(const char* name,
		const MObject::unique_list& args, int timeout)
	{
		auto func = get_global(name);
		if (func->get_type() != Type_Function)
			throw std::exception("lua: Attempted function call on non-function entity.");
		
		func->push(*this);

		// Push the arguments on the stack
		for (auto itr = args.begin(); itr != args.end(); ++itr)	push(**itr);

		// Call the Lua function
		if (lua_pcall_t(L, args.size(), LUA_MULTRET, 0, timeout))
		{
			std::string error = lua_tostring(L, -1);
			lua_pop(L, -1);
			throw std::exception(error.c_str());
		}
		
		MObject::unique_deque results;

		// Pop the results off the stack
		int n = lua_gettop(L);
		for (int i = 0; i < n; i++)	{
			if (lua_type(L, n) == Type_Nil) continue;
			results.push_front(pop()->release_object());
		}

		return results;
	}

	// Calls a function
	// Caller is responsible for memory management of return vector
	MObject::unique_deque State::Call(const char* name, int timeout)
	{
		static const MObject::unique_list args;
		return this->Call(name, args, timeout);
	}
	
	// -------------------------------------------------------------------
	// Used by CFunction for invoking C functions.
	class LuaCallHandler : public Manager::CallHandler
	{
	private:
		int cur_arg;
		Lua::State& luaState;

	protected:
		void __NO_RET RaiseError(const std::string& err) override
		{
			lua_State* L = luaState.GetState();
			return (void)luaL_error(L, err.c_str());
		}

		// Get the next argument for the function, if it's not of the
		// expected type an error should be described and raised through 
		// RaiseError. 
		std::unique_ptr<MObject> GetArgument(Common::obj_type expected) override
		{		
			lua_State* L = luaState.GetState();
			int indx_from_top = (-1 * lua_gettop(L)) + cur_arg;
			int indx = ++cur_arg;

			std::unique_ptr<MObject> obj;
			switch (expected)
			{
			case Common::TYPE_BOOL:
				{
					int b;
					// phasor_legacy: this "fix" is specific to Phasor
					// old versions accepted 0 as boolean false but Lua treats
					// it as boolean true.
					if (lua_type(L, indx) == Type_Number && lua_tonumber(L, indx) == 0) 
						b = 0;
					else
						b = lua_toboolean(L, indx);
					obj.reset(new MObjBool(b != 0));
				} break;
			case Common::TYPE_NUMBER:
				{
					// if it can't be converted luaL_checknumber raises an error
					lua_Number n = luaL_checknumber(L, indx);
					obj.reset(new MObjNumber(n));
				} break;
			case Common::TYPE_STRING:
				{
					// checkstring converts stack value and raises an error
					// if no conversion exists
					luaL_checkstring(L, indx);
					obj.reset(new MObjString(lua_tostring(L, indx)));
				} break;
			case Common::TYPE_TABLE:
				{
					if (lua_type(L, indx) != Type_Table) {
						std::stringstream ss;
						ss << "bad argument #" << cur_arg << " to '" << cb->name
							<< "' (table expected, got " << lua_typename(L, indx) 
							<< ")";
						RaiseError(ss.str());
					}
					obj.reset(luaState.peek_table(indx_from_top).release());
				} break;
			case Common::TYPE_ANY:
				{
					obj = luaState.peek(indx_from_top)->release_object();
				} break;
			}
			return obj;
		}
	public:
		LuaCallHandler(State& state, 
			const Manager::ScriptCallback* cb, int nargs)
			: CallHandler(state, cb, nargs), cur_arg(0), luaState(state)
		{
		}
	};

	// --------------------------------------------------------------------
	//
	LuaObject::LuaObject()
		: type(Type_Nil), object(new MObject())
	{
	}

	LuaObject::LuaObject(bool value)
		: type(Type_Boolean), object(new MObjBool(value))
	{
	}

	LuaObject::LuaObject(double value)
		: type(Type_Number), object(new MObjNumber(value))
	{
	}

	LuaObject::LuaObject(const std::string& value)
		: type(Type_String), object(new MObjString(value))
	{
	}

	LuaObject::LuaObject(const char* value)
		: type(Type_String), object(new MObjString(value))
	{
	}

	LuaObject::LuaObject(LuaType type) 
		: type(type), object(new MObject())
	{
	}

	LuaObject::LuaObject(std::unique_ptr<MObjTable> table)
		: type(Type_Table), object(std::move(table))
	{
	}	

	LuaObject::~LuaObject() {}

	void LuaObject::push(State& state) 
	{
		return state.push(*object);
	}

	std::unique_ptr<MObject> LuaObject::release_object()
	{
		return std::move(object);
	}
	
	// --------------------------------------------------------------------
	//
	UnhandledObject::UnhandledObject(State& state, int ref, LuaType type)
		: LuaObject(type), state(state), ref(ref)
	{
	}

	UnhandledObject::~UnhandledObject()
	{
		luaL_unref(state.GetState(), LUA_REGISTRYINDEX, ref);
	}

	void UnhandledObject::push(State& state)
	{
		if (&state != &this->state) throw std::logic_error("each UnhandledObject can only be bound to one state.");
		lua_rawgeti(state.GetState(), LUA_REGISTRYINDEX, ref);
	}

	// --------------------------------------------------------------------
	//
	CFunction::CFunction(State& state, const Manager::ScriptCallback* cb)
		: LuaObject(Type_Function), state(state), cb(cb)
	{
	}

	void CFunction::push(State& state)
	{
		if (&state != &this->state) throw std::logic_error("each CFunction can only be bound to one state.");
		lua_State* L = state.GetState();
		lua_pushlightuserdata(L, (void*)this);
		lua_pushcclosure(L, CFunction::LuaCall, 1);
	}

	int CFunction::LuaCall(lua_State* L)
	{
		// Get the CFunction class from upvalue
		CFunction* function = (CFunction*)lua_touserdata(L, lua_upvalueindex(1));	

		int nargs = lua_gettop(L); // number of args received
		LuaCallHandler call(function->state, function->cb, nargs);

		// build the argument list + call the func
		MObject::unique_list results = call.Call();
		lua_pop(L, nargs); // done with args now

		// Push the results on the stack
		for (auto itr = results.begin(); itr != results.end(); ++itr)
			function->state.push(*(*itr));

		int nresults = results.size();
		return nresults;
	}
}