#include "Manager.h"
#include "Common.h"
#include "Lua.h"
#include <sstream>

using namespace Common;

namespace Manager
{
	std::unique_ptr<ScriptState> OpenScript(const char* file)
	{	
		//todo: if support other languages, add handlers here
		std::unique_ptr<ScriptState> state(new Lua::State(file));
		return state;
	}

	void CloseScript(std::unique_ptr<ScriptState>& state)
	{
		state.reset();
	}

	// Register the specified functions with the script
	void RegisterFunctions(ScriptState& state, const ScriptCallback* funcs, size_t n)
	{
		for (size_t i = 0; i < n; i++) state.RegisterFunction(&funcs[i]);
	}

	// Scripts should use call this function when invoking a C function
	MObject::unique_list InvokeCFunction(ScriptState& state,
		MObject::unique_deque& args, const ScriptCallback* cb)
	{
		Common::Object::unique_list results;
		cb->func(args, results);
		return results;
	}

	// --------------------------------------------------------------------
	// Class: Result
	// Provides an interface for retrieving values from scripts
	//
	Result::Result()
	{
	}

	void Result::SetData(const Result& other)
	{
		Clear();
		auto itr = other.result.begin();
		while (itr != other.result.end())
		{
			result.push_back((*itr)->NewCopy());
			itr++;
		}
	}

	Result::Result(const Result& other)
	{
		SetData(other);
	}

	void Result::Clear()
	{
		result.clear();
	}

	Result::~Result()
	{
		Clear();
	}

	Result& Result::operator=(const Result& rhs)
	{
		if (this == &rhs) return *this;
		SetData(rhs);
		return *this;
	}

	const Object& Result::operator[](size_t index)
	{
		if (index < 0 || index >= result.size()) {
			std::stringstream err;
			err << "script: attempt to access out of bounds result.";
			throw std::exception(err.str().c_str());
		}
		return *result[index];
	}

	size_t Result::size() const
	{
		return result.size();
	}

	// --------------------------------------------------------------------
	// Class: Caller
	// Provides an interface for passing parameters to scripts.
	// 
	Caller::Caller()
	{
	}

	void Caller::SetData(const Caller& other)
	{
		Clear();
		auto args_itr = other.args.begin();
		while (args_itr != other.args.end())
		{
			args.push_back((*args_itr)->NewCopy());
			args_itr++;
		}
	}

	Caller::Caller(const Caller& other)
	{
		SetData(other);
	}

	void Caller::Clear()
	{
		args.clear();
	}

	Caller::~Caller()
	{
		Clear();
	}

	Caller& Caller::operator=(const Caller& rhs)
	{
		if (this == &rhs) return *this;
		SetData(rhs);
		return *this;
	}

	void Caller::AddArg(bool b)
	{
		std::unique_ptr<Common::Object> arg(new ObjBool(b)); 
		args.push_back(std::move(arg));
	}

	void Caller::AddArg(const char* str)
	{
		std::unique_ptr<Common::Object> arg(new ObjString(str));
		args.push_back(std::move(arg));
	}

	void Caller::AddArg(int value)
	{
		std::unique_ptr<Common::Object> arg(new ObjNumber(value));
		args.push_back(std::move(arg));
	}

	void Caller::AddArg(DWORD value)
	{
		std::unique_ptr<Common::Object> arg(new ObjNumber(value));
		args.push_back(std::move(arg));
	}

	void Caller::AddArg(float value)
	{
		std::unique_ptr<Common::Object> arg(new ObjNumber(value));
		args.push_back(std::move(arg));
	}

	void Caller::AddArg(double value)
	{
		std::unique_ptr<Common::Object> arg(new ObjNumber(value));
		args.push_back(std::move(arg));
	}

	void Caller::AddArg(const std::map<std::string, std::string>& table)
	{
		std::unique_ptr<Common::Object> arg(new ObjTable(table));
		args.push_back(std::move(arg));
	}

	Result Caller::Call(ScriptState& state, const char* function, bool* found, int timeout)
	{
		Result result;
		if (state.HasFunction(function))
		{
			result.result = state.Call(function, args, timeout);
			if (found) *found = true;
		}
		else if(found) *found = false;

		return result;
	}

	Result Caller::Call(ScriptState& state, const char* function, int timeout)
	{
		return Call(state, function, NULL, timeout);
	}
}