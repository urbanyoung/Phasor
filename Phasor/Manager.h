#pragma once

#include <map>
#include <deque>
#include <list>
#include <array>
#include <memory>
#include <stack>
#include "Common.h"
#include "PhasorScript.h" // for ScriptState

typedef unsigned long DWORD;

// Forward declare relevant classes
namespace Lua
{
	class State;
}

namespace Manager
{
	class ScriptState;
}

typedef Manager::ScriptState ScriptState;	

// Namespace: Manager
// Provides an interface for managing scripts of different types.
// Currently only Lua is supported, but others can be integrated easily.
namespace Manager
{		
	class Result;			
	class ScriptState;

	// nice article about aggregate types http://stackoverflow.com/questions/4178175/what-are-aggregates-and-pods-and-how-why-are-they-special
	struct ScriptCallback 
	{
		void (*func)(Common::Object::unique_deque&,	Common::Object::unique_list&);
		const char* name;
		int minargs;
		std::tr1::array<Common::obj_type, 5> fmt; // change max args as needed
	};

	typedef Common::Object MObject;
	typedef Common::ObjBool MObjBool;
	typedef Common::ObjNumber MObjNumber;
	typedef Common::ObjString MObjString;
	typedef Common::ObjTable MObjTable;

	typedef Common::obj_type obj_type;

	// --------------------------------------------------------------------

	// Opens the script specified, relative to the scripts directory
	// May throw an exception <todo: add specific info>
	std::unique_ptr<ScriptState> OpenScript(const char* file);

	void CloseScript(std::unique_ptr<ScriptState>& state);

	// Register the specified functions with the script
	void RegisterFunctions(ScriptState& state, const ScriptCallback* funcs, size_t n);

	// Scripts should use call this function when invoking a C function
	MObject::unique_list InvokeCFunction(ScriptState& state,
		MObject::unique_deque & args, const ScriptCallback* cb);

	struct ScriptCallstack
	{
		std::string func;
		bool scriptInvoked; // false: called script, true script called

		ScriptCallstack(const std::string& func, bool scriptInvoked)
			: func(func), scriptInvoked(scriptInvoked) {}
	};

	// --------------------------------------------------------------------
	// Class: ScriptState
	// Classes compatible with this interface should inherit this class.
	class ScriptState : public Scripting::PhasorScript
	{
	public:
		virtual ~ScriptState(){}

		std::stack<std::unique_ptr<ScriptCallstack>> callstack;

		void PushCall(const std::string& func, bool scriptInvoked);
		void PopCall();

		// Checks if the specified function is defined in the script
		virtual bool HasFunction(const char* name) = 0;

		// Registers a C function with the script
		virtual void RegisterFunction(const ScriptCallback* cb) = 0;

		// Calls a function with an optional timeout
		virtual MObject::unique_deque Call(const char* name,
			const MObject::unique_list& args, int timeout = 0) = 0;
		virtual MObject::unique_deque Call(const char* name, int timeout = 0) = 0;
	};

	// --------------------------------------------------------------------
	// Class: Result
	// Provides an interface for retrieving values from scripts
	class Result
	{
	protected:
		
		Common::Object::unique_deque result;

		// Copy the other result into this one
		void SetData(const Result& other);

		// Clear the current data
		void Clear();

	public:
		Result();
		Result(const Result& other);
		Result& operator=(const Result& rhs);
		~Result();

		// Returns number of items stored
		size_t size() const;

		// Gets item at specified position (may throw exception)
		const Common::Object& operator [] (size_t index);

		friend class Caller;
	};

	// --------------------------------------------------------------------
	// Class: Caller
	// Provides an interface for passing parameters to scripts.
	class Caller
	{
	protected:
		Common::Object::unique_list args;

		// Copy other into this object
		void SetData(const Caller& other);			

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

		// Clears the current argument list
		void Clear();

		// Calls the specified function on the specified script.
		Result Call(ScriptState& state, const std::string& function, bool* found, int timeout);
		Result Call(ScriptState& state, const std::string& function, int timeout);
	};
}

