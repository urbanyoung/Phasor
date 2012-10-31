#include "Scripting.h"
#include <map>
#include <string>
#include <sstream>
#include <windows.h> // for GetCurrentProcessId()

// todo: overload Call for no args
// check what happens if attempt to call non existant lua function
// 
namespace Scripting
{
	#define exception_type std::exception // todo: change to custom type
	#define DEFAULT_TIMEOUT 2000
	// Use to ensure the default timeout is always specified
	#define SCRIPT_CALL(script, name, args) \
		script->Call(name, args, DEFAULT_TIMEOUT)

	const int versions[] = {1234};

	std::string scriptsDir;
	std::map<std::string, Lua::State*> scripts;

	struct PhasorAPI
	{
		std::vector<Lua::Object*> (*func)(Lua::State*, std::vector<Lua::Object*>&);
		const char* name;
	};

	std::vector<Lua::Object*> testf(Lua::State*, std::vector<Lua::Object*>&)
	{
		return std::vector<Lua::Object*>();
	}

	PhasorAPI PhasorExportTable[] =
	{
		{&testf, "updateammo"}
	};

	// --------------------------------------------------------------------
	//
	
	void SetPath(const char* scriptPath)
	{
		scriptsDir = scriptPath;
	}

	void RegisterFunctions(Lua::State* state)
	{
		const int n = sizeof(PhasorExportTable)/sizeof(PhasorAPI);
		for (int i = 0; i < n; i++)
			state->RegisterFunction(PhasorExportTable[i].name, PhasorExportTable[i].func);
	}

	bool CompatibleVersion(int required) 
	{
		const int n = sizeof(versions)/sizeof(int);
		bool compatible = false;
		for (int i = 0; i < n; i++)
		{
			if (versions[i] == required) {
				compatible = true;
				break;
			}
		}
		return compatible;		
	}

	void OpenScript(const char* script)
	{
		// Make sure the script isn't already loaded
		if (scripts.find(script) != scripts.end()) {
			std::stringstream err;
			err << "script: " << script << " is already loaded.";
			throw std::exception(err.str().c_str());
		}
		std::stringstream abs_file;
		abs_file << scriptsDir << "\\" << script;

		Lua::State* state = Lua::State::NewState();

		try 
		{
			state->DoFile(abs_file.str().c_str());

			// Register all functions with the vm
			RegisterFunctions(state);

			std::vector<Lua::Object*> args;
			std::vector<Lua::Object*> vals = SCRIPT_CALL(state, "GetRequiredVersion", args);

			// Make sure the requested version is compatible
			bool is_compatible = false;
			if (vals.size() == 1) {
				Lua::Number* ver = (Lua::Number* )vals[0];
				if (ver->GetType() == Lua::Type_Number &&
					CompatibleVersion(ver->GetValue())) {
						is_compatible = true;
				}
			}

			if (!is_compatible) {
				std::stringstream err;
				err << "script: " << script << " is not compatible with this version of Phasor.";
				throw std::exception(err.str().c_str());
			}

			args.push_back(state->NewNumber(GetCurrentProcessId()));
			SCRIPT_CALL(state, "OnScriptLoad", args);

			scripts[script] = state;

		} catch (exception_type) 
		{
			Lua::State::Close(state);
			throw;
		}	
	}

	void CloseScript(const char* script) 
	{
		// todo: Check how Call can fail and make sure all failures
		// are handled here.
		std::map<std::string, Lua::State*>::iterator itr = scripts.find(script);

		if (itr != scripts.end()) {
			try 
			{
				std::vector<Lua::Object*> args;
				SCRIPT_CALL(itr->second, "OnScriptUnload", args);
			}
			catch (exception_type) 
			{}
			Lua::State::Close(itr->second);			
		}
	}

	void Call(const char* function, const std::vector<Lua::Object*>& args)
	{

	}
}