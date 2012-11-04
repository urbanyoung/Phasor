#include "Scripting.h"
#include "Manager.h"
#include <string>
#include <sstream>
#include <windows.h> // for GetCurrentProcessId()

// todo: overload Call for no args
// check what happens if attempt to call non existent lua function
// 
namespace Scripting
{
	using namespace Common;

	#define exception_type std::exception // todo: change to custom type
	#define DEFAULT_TIMEOUT 2000

	const DWORD versions[] = {1234};

	std::string scriptsDir;

	/*struct PhasorAPI
	{
		std::vector<Object*> (*func)(ScriptState*, std::vector<Object*>&);
		const char* name;
	};

	std::vector<Object*> testf(ScriptState*, std::vector<Object*>&)
	{
		return std::vector<Object*>();
	}

	PhasorAPI PhasorExportTable[] =
	{
		{&testf, "updateammo"}
	};*/

	// Registers the Phasor API with scripts
	void RegisterFunctions(ScriptState* state);

	// Checks if the script is compatible with this version of Phasor.
	void CheckScriptCompatibility(Lua::State* state, const char* script);

	// --------------------------------------------------------------------
	//
	void OpenScript(const char* script)
	{
		std::stringstream abs_file;
		abs_file << scriptsDir << "\\" << script << ".lua";

		try 
		{
			ScriptState* state = Manager::OpenScript(abs_file.str().c_str(), script);
			
			//RegisterFunctions(state);
			CheckScriptCompatibility(state, script);			

			// Notify the script that it's been loaded.
			bool fexists = false;
			Manager::Caller caller;
			caller.AddArg(GetCurrentProcessId());
			caller.Call(state, "OnScriptLoad", &fexists, DEFAULT_TIMEOUT);

			if (!fexists) {
				std::stringstream err;
				err << "script: " << script << " cannot be loaded, OnScriptLoad undefined.";
				throw std::exception(err.str().c_str());
			}
		}
		catch (exception_type) 
		{
			CloseScript(script);
			throw;
		}	
	}

	void CloseScript(const char* script) 
	{
		ScriptState* state = Manager::FindScript(script);

		if (state) {
			try 
			{
				Manager::Caller caller;
				caller.Call(state, "OnScriptUnload", DEFAULT_TIMEOUT);
				Manager::CloseScript(script);
			}
			catch (exception_type) 
			{
				Manager::CloseScript(script);
				throw;
			}					
		}
	}

	// Sets the path to be used by this namespace (where scripts are).
	void SetPath(const char* scriptPath)
	{
		scriptsDir = scriptPath;
	}

	// Registers the Phasor API with scripts
	/*void RegisterFunctions(Lua::State* state)
	{
		const int n = sizeof(PhasorExportTable)/sizeof(PhasorAPI);
		for (int i = 0; i < n; i++)
			state->RegisterFunction(PhasorExportTable[i].name, PhasorExportTable[i].func);
	}*/

	// Checks if the specified version is compatible
	bool CompatibleVersion(DWORD required) 
	{
		const int n = sizeof(versions)/sizeof(versions[0]);
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

	// Checks if the script is compatible with this version of Phasor.
	void CheckScriptCompatibility(ScriptState* state, const char* script) 
	{
		bool funcExists = false;
		Manager::Caller caller;
		Result result = caller.Call(state, "GetRequiredVersion", &funcExists, DEFAULT_TIMEOUT);

		if (!funcExists) {
			std::stringstream err;
			err << "script: " << script << " cannot be loaded, GetRequiredVersion undefined.";
			throw std::exception(err.str().c_str());
		}

		// Make sure the requested version is compatible
		bool is_compatible = false;
		if (result.size() == 1 && result[0]->GetType() == TYPE_NUMBER) {
			ObjNumber* ver = (ObjNumber*)result[0];
			if (CompatibleVersion((DWORD)ver->GetValue())) {
					is_compatible = true;
			}
		}

		if (!is_compatible) {
			std::stringstream err;
			err << "script: " << script << " is not compatible with this version of Phasor.";
			throw std::exception(err.str().c_str());
		}
	}

	// --------------------------------------------------------------------
	// 
	Result PhasorCaller::Call(const char* function)
	{
		// This is the argument which indicates if a scripts' return 
		// value is used.
		this->AddArg(true);
		ObjBool* using_param = (ObjBool*)*args.rbegin();

		std::map<std::string, ScriptState*> scripts = Manager::GetScripts();
		std::map<std::string, ScriptState*>::const_iterator itr = scripts.begin();

		Result result;
		bool result_set = false;

		while (itr != scripts.end())
		{
			ScriptState* state = itr->second;
			bool found = false;
			Result r = Manager::Caller::Call(state, function, &found, DEFAULT_TIMEOUT);

			// The first result with any non-nil values is used
			if (found && !result_set) {
				for (size_t i = 0; i < r.size(); i++) {
					if (r[i]->GetType() != TYPE_NIL) {
						result = r;
						result_set = true;

						// Change the 'using' argument so other scripts know
						// they won't be considered
						*using_param = false;
						break;
					}
				}
			}
		
			itr++;
		}

		this->Free();

		return result;
	}
}