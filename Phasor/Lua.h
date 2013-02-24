#pragma once

#include <list>
#include <string>
#include <deque>
#include <memory>
#include "..\lua\lua.hpp"
#include "Manager.h"
#include "Common/noncopyable.h"

namespace Lua
{
	typedef Manager::MObject MObject;
	typedef Manager::MObjBool MObjBool;
	typedef Manager::MObjNumber MObjNumber;
	typedef Manager::MObjString MObjString;
	typedef Manager::MObjTable MObjTable;

	class State;

	enum LuaType
	{
		Type_Nil = LUA_TNIL,
		Type_Boolean = LUA_TBOOLEAN,
		Type_LightUserData = LUA_TLIGHTUSERDATA,
		Type_Number = LUA_TNUMBER,
		Type_String = LUA_TSTRING,
		Type_Table = LUA_TTABLE,
		Type_Function = LUA_TFUNCTION,
		Type_UserData = LUA_TUSERDATA,
		Type_Thread = LUA_TTHREAD,
	};

	class LuaObject : noncopyable
	{
	private:
		std::unique_ptr<MObject> object;

	protected:
		LuaType type;

		explicit LuaObject(LuaType type);
	public:
		explicit LuaObject(); // nil
		explicit LuaObject(bool value);
		explicit LuaObject(double value);
		explicit LuaObject(const std::string& value);
		explicit LuaObject(const char* value);
		explicit LuaObject(int index, bool); // use for unknown types
		explicit LuaObject(std::unique_ptr<MObjTable> table);
		virtual ~LuaObject();

		LuaType get_type() { return type;}

		virtual void push(State& state);
		std::unique_ptr<MObject> release_object();
	};

	class UnhandledObject : public LuaObject
	{
	private: 
		int ref;
		State& state;

	public:
		explicit UnhandledObject(State& state, int ref, LuaType actual_type);
		~UnhandledObject();
		virtual void push(State& state);
	};

	// Class: CFunction
	// used to represent a c function which is callable from Lua. The same
	// CFunction cannot be bound to multiple states.
	class CFunction : public LuaObject
	{
	private:

		State& state;
		const Manager::ScriptCallback* cb;

	public:
		CFunction(State& state, const Manager::ScriptCallback* cb);
		// inherited from LuaObject
		virtual void push(State&) override;

		static int LuaCall(lua_State* L);
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

		State();
		virtual ~State();	

		lua_State* GetState() { return L; }

		void DoFile(const char* file) override;

		// Loads and runs a string
		void DoString(const char* str);

		// Push handled types to the stack (those that MObject can use)
		void push(const MObject& object);
		std::unique_ptr<LuaObject> peek(int indx=-1);
		std::unique_ptr<LuaObject> pop();
		std::unique_ptr<MObjTable> peek_table(int indx=-1);

		std::unique_ptr<LuaObject> get_global(const char* name);
		void set_global(const char* name, LuaObject& obj);		

		// inherited from Manager::ScriptState
		void RegisterFunction(const Manager::ScriptCallback* cb);
		bool HasFunction(const char* name);
		MObject::unique_deque Call(const char* name,
			const MObject::unique_list& args, int timeout = 0);
		MObject::unique_deque Call(const char* name, int timeout = 0);
	};
}
