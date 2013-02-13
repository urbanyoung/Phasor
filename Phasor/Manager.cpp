#include "Manager.h"
#include "Common/Common.h"
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

	// Used for invoking c functions from scripts
	static MObject::unique_list InvokeCFunction(ScriptState& state,
		CallHandler& handler, MObject::unique_deque& args, const ScriptCallback* cb)
	{
		Common::Object::unique_list results;
		state.PushCall(cb->name, true);
		cb->func(handler, args, results);
		state.PopCall();
		return results;
	}

	// --------------------------------------------------------------------
	// Class: CallHandler
	CallHandler::CallHandler(ScriptState& state, const ScriptCallback* cb, int nargs)
			: state(state), cb(cb), nargs(nargs) 
	{
	}

	MObject::unique_list CallHandler::Call()
	{
		static const int maxargs_all = cb->fmt.size(); // max any function can receive
		int maxargs = 0; // max number of arguments this specific function expects
		int minargs = cb->minargs; // minimum allowed

		// Make sure the received arguments are within the extreme limits
		if (nargs < minargs) { 
			std::stringstream ss;
			ss << "'" << cb->name << "' expects at least " << minargs  
				<< " argument(s) and received " << nargs;
			__NO_RET RaiseError(ss.str());
		} else if (nargs > maxargs_all) {
			std::stringstream ss;
			ss << "'" << cb->name << "' expects at most " << maxargs_all  
				<< " argument(s) and received " << nargs;
			__NO_RET RaiseError(ss.str());
		}

		MObject::unique_deque args;	
		for (int i = 0; i < nargs; i++) {
			if (cb->fmt[i] == Common::TYPE_NIL) break;
			maxargs++;
			args.push_back(GetArgument(cb->fmt[i]));
		}
		if (maxargs < nargs) {
			std::stringstream ss;
			ss << "'" << cb->name << "' only expects " << maxargs <<
				" argument(s) and received " << nargs;
			__NO_RET RaiseError(ss.str());
		}

		return InvokeCFunction(state, *this, args, cb);
	}

	// --------------------------------------------------------------------
	// Class: ScriptState
	void ScriptState::PushCall(const std::string& func, bool scriptInvoked)
	{
		callstack.push(std::unique_ptr<ScriptCallstack>(new ScriptCallstack(func, scriptInvoked)));
	}

	void ScriptState::PopCall()
	{
		if (!callstack.empty()) callstack.pop();
	}

	// --------------------------------------------------------------------
	// Class: Result
	// Provides an interface for retrieving values from scripts
	//
	Result::Result() : index(0)
	{
	}

	Result::Result(const Result& other) : index(0)
	{
		SetData(other);
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

	void Result::Replace(size_t index, std::unique_ptr<Common::Object> obj)
	{
		result[index] = std::move(obj);
	}

	obj_type Result::GetType(size_t index)
	{
		return result[index]->GetType();
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

	void Caller::AddArgNil()
	{
		std::unique_ptr<Common::Object> arg(new Object()); 
		args.push_back(std::move(arg));
	}
	void Caller::AddArg(bool b)
	{
		std::unique_ptr<Common::Object> arg(new ObjBool(b)); 
		args.push_back(std::move(arg));
	}

	void Caller::AddArg(const std::string& str)
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

	Result Caller::Call(ScriptState& state, const std::string& function, bool* found, int timeout)
	{
		Result result;
		if (state.HasFunction(function.c_str()))
		{
			state.PushCall(function, false);
			result.result = state.Call(function.c_str(), args, timeout);
			state.PopCall();
			if (found) *found = true;
		}
		else if(found) *found = false;
		return result;
	}

	Result Caller::Call(ScriptState& state, const std::string& function, int timeout)
	{
		return Call(state, function, NULL, timeout);
	}
}