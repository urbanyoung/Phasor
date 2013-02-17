#pragma once

#include <map>
#include <deque>
#include <list>
#include <array>
#include <memory>
#include <stack>
#include "Common/Common.h"

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
	class CallHandler;

	// nice article about aggregate types http://stackoverflow.com/questions/4178175/what-are-aggregates-and-pods-and-how-why-are-they-special
	struct ScriptCallback 
	{
		void (*func)(CallHandler& handler, Common::Object::unique_deque&,
			Common::Object::unique_list&);
		const char* name;
		int minargs;
		std::array<Common::obj_type, 5> fmt; // change max args as needed
	};

	typedef Common::Object MObject;
	typedef Common::ObjBool MObjBool;
	typedef Common::ObjNumber MObjNumber;
	typedef Common::ObjString MObjString;
	typedef Common::ObjTable MObjTable;

	typedef Common::obj_type obj_type;

	// --------------------------------------------------------------------

	// Creates a new script, doesn't load any files.
	std::unique_ptr<ScriptState> CreateScript();

	void CloseScript(std::unique_ptr<ScriptState>& state);

	// Register the specified functions with the script
	void RegisterFunctions(ScriptState& state, const ScriptCallback* funcs, size_t n);

	struct ScriptCallstack
	{
		std::string func;
		bool scriptInvoked; // false: called script, true script called

		ScriptCallstack(const std::string& func, bool scriptInvoked)
			: func(func), scriptInvoked(scriptInvoked) {}
	};

#define __NO_RET
	// Scripts should derive from this and implement the getters.
	class CallHandler
	{
	private:
		const ScriptCallback* cb;
		int nargs;		

	protected:

		// Get the next argument for the function, if it's not of the
		// expected type an error should be described and raised through 
		// RaiseError. 
		virtual std::unique_ptr<MObject> GetArgument(Common::obj_type expected) = 0;

		CallHandler(ScriptState& state, const ScriptCallback* cb, int nargs);
	public:		

		ScriptState& state;

		// Invokes the call to the setup function
		MObject::unique_list Call();

		// This function should propagate the error to the script's
		// virtual machine. It should not return. 
		virtual void __NO_RET RaiseError(const std::string& err) = 0;
	};

	// --------------------------------------------------------------------
	// Class: ScriptState
	// Classes compatible with this interface should inherit this class.
	class ScriptState
	{
	public:
		virtual ~ScriptState(){}

		std::stack<std::unique_ptr<ScriptCallstack>> callstack;

		void PushCall(const std::string& func, bool scriptInvoked);
		void PopCall();

		// Loads the specified file
		virtual void DoFile(const char* file) = 0;

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
	private:
		size_t index;

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

		// Read the next result
		template <class T>
		T& ReadResult() { return (T&)*result[index++];}

		MObject& ReadObject() { return ReadResult<MObject>(); }
		MObjBool& ReadBool() { return ReadResult<MObjBool>(); }
		MObjNumber& ReadNumber() { return ReadResult<MObjNumber>(); }
		MObjString& ReadString() { return ReadResult<MObjString>(); }
		MObjTable& ReadTable() { return ReadResult<MObjTable>(); }

		obj_type GetType(size_t index);
		void Replace(size_t index, std::unique_ptr<Common::Object> obj);

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
		void AddArgNil();
		void AddArg(bool b);
		void AddArg(const std::string& str);
		void AddArg(int value);
		void AddArg(DWORD value);
		void AddArg(float value);
		void AddArg(double value);	
		void AddArg(const std::map<std::string, std::string>& table);	
		void AddArg(std::unique_ptr<Common::Object> obj);

		// Clears the current argument list
		void Clear();

		// Calls the specified function on the specified script.
		Result Call(ScriptState& state, const std::string& function, bool* found, int timeout);
		Result Call(ScriptState& state, const std::string& function, int timeout);
	};
}

