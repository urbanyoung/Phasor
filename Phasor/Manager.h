#pragma  once

#include <map>
#include <deque>
#include <list>
#include <array>

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

	enum obj_type;
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

	// nice article about aggregate types http://stackoverflow.com/questions/4178175/what-are-aggregates-and-pods-and-how-why-are-they-special
	struct ScriptCallback 
	{
		void (*func)(std::deque<Common::Object*>&, std::list<Common::Object*>&);
		const char* name;
		int minargs;
		std::tr1::array<Common::obj_type, 5> fmt; // change max args as needed
	};

	// --------------------------------------------------------------------

	// Opens the script specified, relative to the scripts directory
	// May throw an exception <todo: add specific info>
	ScriptState* OpenScript(const char* file);

	// Register the specified functions with the script
	void RegisterFunctions(ScriptState* state, const ScriptCallback* funcs, size_t n);

	// Scripts should use call this function when invoking a C function
	std::list<MObject*> InvokeCFunction(ScriptState* state, const std::deque<MObject*>& args, const ScriptCallback* cb);

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

		virtual void RegisterFunction(const ScriptCallback* cb) = 0;

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
		// For use by Manager
	public:
	
		// Converts state-bound objects to generic ones.
		static void ConvertFromState(const std::deque<MObject*>& in, std::deque<Common::Object*>& out);

		// Converts generic objects to state-bound ones.
		static void ConvertToState(ScriptState* state, const std::list<Common::Object*>& in, std::list<MObject*>& out);
		static void ConvertToState(ScriptState* state, const std::list<Common::Object>& in, std::list<MObject*>& out);

		// Frees state-bound objects
		static void FreeStateBound(std::deque<MObject*>& in);
		static void FreeStateBound(std::list<MObject*>& in);

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

		// Copy the other result into this one
		void SetData(const Result& other);

		// Clear the current data
		void Clear();

		// Constructs the result and takes ownership of the memory.
		Result(const std::deque<Common::Object*>& result);

	public:
		Result();
		Result(const Result& other);
		Result& operator=(const Result& rhs);
		~Result();

		// Returns number of items stored
		size_t size() const;

		// Gets item at specified position (may throw exception)
		const Common::Object* operator [] (size_t index);

		friend class Caller;
	};

	// --------------------------------------------------------------------
	// Class: Caller
	// Provides an interface for passing parameters to scripts.
	class Caller
	{
	protected:
		std::list<Common::Object*> args;

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
		Result Call(ScriptState* state, const char* function, bool* found, int timeout);
		Result Call(ScriptState* state, const char* function, int timeout);
	};
}
