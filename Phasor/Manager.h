#pragma  once

#include <map>
#include <deque>
#include <list>

typedef unsigned long DWORD;

// Forward declare relevant classes
namespace Lua
{
	class State;
}

namespace Common
{
	class Object;
	class ObjBool;
	class ObjNumber;
	class ObjString;
	class ObjTable;
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
	class MObject;

	// --------------------------------------------------------------------

	// Opens the script specified, relative to the scripts directory
	// May throw an exception <todo: add specific info>
	ScriptState* OpenScript(const char* file);

	// Closes the specified script
	void CloseScript(ScriptState* state);

	// --------------------------------------------------------------------
	// Class: ScriptState
	// Classes compatible with this interface should inherit this class.
	class ScriptState
	{
	public:
		virtual MObject* ToNativeObject(const Common::Object* in) = 0; 

		// Checks if the specified function is defined in the script
		virtual bool HasFunction(const char* name) = 0;

		// Calls a function with an optional timeout
		// Caller is responsible for memory management of return vector
		virtual std::deque<MObject*> Call(const char* name, const std::list<MObject*>& args, int timeout = 0) = 0;
		virtual std::deque<MObject*> Call(const char* name, int timeout = 0) = 0;
	};

	// --------------------------------------------------------------------
	// Class: MObject
	// Classes compatible with this interface should return objects derived
	// from this type and provide conversions to Common::Object and its derivatives
	class MObject
	{
	public:
		// All objects should be allocated via 'new'.
		virtual Common::Object* ToGeneric() const = 0;

		// Called when an object should delete itself
		virtual void Delete() = 0;
		
	};

	// --------------------------------------------------------------------
	// Class: Result
	// Provides an interface for retrieving values from scripts
	class Result
	{
	protected:
		std::deque<Common::Object*> result;

		void SetData(const Result& other);
		void Free();

		// Constructs the result and takes ownership of the memory.
		Result(const std::deque<Common::Object*>& result);

	public:
		Result();
		Result(const Result& other);
		Result& operator=(const Result& rhs);
		~Result();

		size_t size() const;

		const Common::Object* operator [] (size_t index);

		friend class Caller;
	};

	// --------------------------------------------------------------------
	// Class: Caller
	// Provides an interface for passing parameters to scripts.
	class Caller
	{
	private:
		// Converts state-bound objects to generic ones.
		void ConvertFromState(const std::deque<MObject*>& in, std::deque<Common::Object*>& out);

		// Converts generic objects to state-bound ones.
		void ConvertToState(ScriptState* state, const std::list<Common::Object*>& in, std::list<MObject*>& out);

		// Frees state-bound objects
		void FreeStateBound(std::deque<MObject*>& in);
		void FreeStateBound(std::list<MObject*>& in);

	protected:
		std::list<Common::Object*> args;

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

		Result Call(ScriptState* state, const char* function, bool* found, int timeout);
		Result Call(ScriptState* state, const char* function, int timeout);
	};
}
