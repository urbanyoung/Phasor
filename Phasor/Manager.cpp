#include "Manager.h"
#include "Common.h"
#include "Lua.h"
#include <sstream>

using namespace Common;

namespace Manager
{
	ScriptState* OpenScript(const char* file)
	{	
		//todo: if support other languages, add handlers here
		Lua::State* state = Lua::State::NewState();

		try 
		{
			state->DoFile(file);			
		}
		catch (std::exception) 
		{
			Lua::State::Close(state);
			throw;
		}	

		return state;
	}

	void CloseScript(ScriptState* state)
	{
		//todo: if support other languages, add handlers here
		Lua::State::Close((Lua::State*)state);
	}

	// Register the specified functions with the script
	void RegisterFunctions(ScriptState* state, const ScriptCallback* funcs, size_t n)
	{
		// todo: if support other languages, add handlers here.
		for (size_t i = 0; i < n; i++)
		{
			state->RegisterFunction(&funcs[i]);
		}
	}

	// Scripts should use call this function when invoking a C function
	std::list<MObject*> InvokeCFunction(ScriptState* state, 
		const std::deque<MObject*>& args, const ScriptCallback* cb)
	{
		// todo: if support other languages, add handlers here.
		std::deque<Common::Object*> c_args;
		MObject::ConvertFromState(args, c_args);

		std::list<Common::Object*> c_results;
		cb->func(c_args, c_results);

		std::list<MObject*> results;
		MObject::ConvertToState(state, c_results, results);

		std::list<Common::Object*>::iterator itr = c_results.begin();
		while (itr != c_results.end())
		{
			delete *itr;
			itr++;
		}
		c_results.clear();

		std::deque<Common::Object*>::iterator itr1 = c_args.begin();
		while (itr1 != c_args.end())
		{
			delete *itr1;
			itr1++;
		}
		return results;
	}

	// --------------------------------------------------------------------
	//
	// 

	void MObject::ConvertFromState(const std::deque<MObject*>& in, std::deque<Common::Object*>& out)
	{
		std::deque<MObject*>::const_iterator itr = in.begin();

		while (itr != in.end())
		{
			out.push_back((*itr)->ToGeneric());
			itr++;
		}
	}

	void MObject::ConvertToState(ScriptState* state, const std::list<Common::Object*>& in, std::list<MObject*>& out)
	{
		std::list<Common::Object*>::const_iterator itr = in.begin();

		while (itr != in.end())
		{
			out.push_back(state->ToNativeObject(*itr));
			itr++;
		}
	}

	void MObject::ConvertToState(ScriptState* state, const std::list<Common::Object>& in, std::list<MObject*>& out)
	{
		std::list<Common::Object>::const_iterator itr = in.begin();

		while (itr != in.end())
		{
			out.push_back(state->ToNativeObject(&(*itr)));
			itr++;
		}
	}

	void MObject::FreeStateBound(std::deque<MObject*>& in)
	{
		std::deque<MObject*>::iterator itr = in.begin();
		while (itr != in.end())
		{
			(*itr)->Delete();
			itr++;
		}
		in.clear();
	}

	void MObject::FreeStateBound(std::list<MObject*>& in)
	{
		std::list<MObject*>::iterator itr = in.begin();
		while (itr != in.end())
		{
			(*itr)->Delete();
			itr++;
		}
		in.clear();
	}

	// --------------------------------------------------------------------
	// Class: Result
	// Provides an interface for retrieving values from scripts
	//
	Result::Result()
	{

	}

	Result::Result(const std::deque<Object*>& result) : result(result)
	{
	}

	void Result::SetData(const Result& other)
	{
		Clear();
		std::deque<Object*>::const_iterator itr = other.result.begin();
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
		std::deque<Object*>::iterator itr = result.begin();
		while (itr != result.end())
		{
			delete *itr;
			itr++;
		}
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

	const Object* Result::operator[](size_t index)
	{
		if (index < 0 || index >= result.size()) {
			std::stringstream err;
			err << "script: attempt to access out of bounds result.";
			throw std::exception(err.str().c_str());
		}
		return result[index];
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
		std::list<Object*>::const_iterator args_itr = other.args.begin();
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
		std::list<Object*>::iterator args_itr = args.begin();
		while (args_itr != args.end())
		{
			delete *args_itr;
			args_itr = args.erase(args_itr);
		}

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
		args.push_back(new ObjBool(b));
	}

	void Caller::AddArg(const char* str)
	{
		args.push_back(new ObjString(str));
	}

	void Caller::AddArg(int value)
	{
		args.push_back(new ObjNumber(value));
	}

	void Caller::AddArg(DWORD value)
	{
		args.push_back(new ObjNumber(value));
	}

	void Caller::AddArg(float value)
	{
		args.push_back(new ObjNumber(value));
	}

	void Caller::AddArg(double value)
	{
		args.push_back(new ObjNumber(value));
	}

	void Caller::AddArg(const std::map<std::string, std::string>& table)
	{
		args.push_back(new ObjTable(table));
	}

	Result Caller::Call(ScriptState* state, const char* function, bool* found, int timeout)
	{
		// todo: make more efficient w.r.t object copying
		Result result = Result();
		if (state->HasFunction(function))
		{
			std::list<MObject*> state_args;
			MObject::ConvertToState(state, args, state_args);

			std::deque<MObject*> state_results = state->Call(function, state_args, timeout);

			std::deque<Common::Object*> results;
			MObject::ConvertFromState(state_results, results);

			result = results;
			*found = true;

			// Free the state-bound stuff
			MObject::FreeStateBound(state_results);
			MObject::FreeStateBound(state_args);
		}
		else *found = false;

		return result;
	}

	Result Caller::Call(ScriptState* state, const char* function, int timeout)
	{
		bool found = false;
		return Call(state, function, &found, timeout);
	}
}