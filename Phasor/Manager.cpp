#include "Manager.h"
#include "Common.h"
#include "Lua.h"
#include <sstream>
#include <map>

using namespace Common;

namespace Manager
{
	std::map<std::string, ScriptState*> scripts;

	ScriptState* OpenScript(const char* file, const char* script_id)
	{
		// Make sure the script isn't already loaded
		if (FindScript(script_id)) {
			std::stringstream err;
			err << "script: " << script_id << " is already loaded.";
			throw std::exception(err.str().c_str());
		}

		Lua::State* state = Lua::State::NewState();

		try 
		{
			state->DoFile(file);
			scripts[script_id] = state;
		}
		catch (std::exception) 
		{
			Lua::State::Close(state);
			throw;
		}	

		return state;
	}

	void CloseScript(const char* script_id)
	{
		std::map<std::string, ScriptState*>::iterator itr = scripts.find(script_id);
		if (itr != scripts.end())
		{
			Lua::State::Close(itr->second);
			scripts.erase(itr);
		}
	}

	ScriptState* FindScript(const char* script_id)
	{
		std::map<std::string, ScriptState*>::iterator itr = scripts.find(script_id);
		return itr == scripts.end() ? NULL : itr->second;
	}

	const std::map<std::string, ScriptState*>& GetScripts()
	{
		return scripts;
	}

	// --------------------------------------------------------------------
	// Class: Result
	// Provides an interface for retrieving values from scripts
	//
	Result::Result()
	{

	}

	Result::Result(const std::vector<Object*>& result) : result(result)
	{
	}

	void Result::SetData(const Result& other)
	{
		std::vector<Object*>::const_iterator itr = other.result.begin();
		result.reserve(other.result.size());
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

	void Result::Free()
	{
		std::vector<Object*>::iterator itr = result.begin();
		while (itr != result.end())
		{
			delete *itr;
			itr++;
		}
		result.clear();
	}

	Result::~Result()
	{
		Free();
	}

	Result& Result::operator=(const Result& rhs)
	{
		if (this == &rhs) return *this;
		Free();
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

	void Caller::Free()
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
		Free();
	}

	Caller& Caller::operator=(const Caller& rhs)
	{
		if (this == &rhs) return *this;
		Free();
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
		Result result = Result();
		if (state->HasFunction(function))
		{
			result = state->Call(function, args, timeout);
			*found = true;
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