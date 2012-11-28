#include "Scripting.h"
#include "Manager.h"
#include "Common.h"
#include "PhasorAPI.h"
#include "PhasorScript.h"
#include <string>
#include <sstream>
#include <windows.h> // for GetCurrentProcessId()
#include <array>

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
	std::map<std::string, std::unique_ptr<ScriptState>> scripts;

	// Called when an error is raised by a script.
	void HandleError(ScriptState& state, std::exception & e);

	// Checks if the script is compatible with this version of Phasor.
	void CheckScriptCompatibility(ScriptState& state, const char* script);

	// --------------------------------------------------------------------
	//
	void OpenScript(const char* script)
	{
		// Make sure the script isn't already loaded
		if (scripts.find(script) != scripts.end()) {
			std::stringstream err;
			err << "script: " << script << " is already loaded.";
			throw std::exception(err.str().c_str());
		}

		std::stringstream abs_file;
		abs_file << scriptsDir << "\\" << script << ".lua";

		std::unique_ptr<ScriptState> state = Manager::OpenScript(abs_file.str().c_str());
			
		CheckScriptCompatibility(*state, script);	
		PhasorAPI::Register(*state);

		// Notify the script that it's been loaded.
		bool fexists = false;
		Manager::Caller caller;
		caller.AddArg(GetCurrentProcessId());
		caller.Call(*state, "OnScriptLoad", &fexists, DEFAULT_TIMEOUT);

		if (!fexists) {
			std::stringstream err;
			err << "script: " << script << " cannot be loaded, OnScriptLoad undefined.";
			throw std::exception(err.str().c_str());
		}

		scripts[script] = std::move(state);

	}

	void CloseScript(const char* script) 
	{
		auto itr = scripts.find(script);

		if (itr != scripts.end()) {
			std::unique_ptr<ScriptState> state(std::move(itr->second));
			scripts.erase(itr);
			
			Manager::Caller caller;
			caller.Call(*state, "OnScriptUnload", DEFAULT_TIMEOUT);
			
			//Manager::CloseScript(state);		
			// script closed when state goes out of scope
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
	void CheckScriptCompatibility(ScriptState& state, const char* script) 
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
		if (result.size() == 1 && result[0].GetType() == TYPE_NUMBER) {
			const ObjNumber& ver = (const ObjNumber&)result[0];
			if (CompatibleVersion((DWORD)ver.GetValue())) {
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
	Result PhasorCaller::Call(const std::string& function)
	{
		// This is the argument which indicates if a scripts' return value is used.
		this->AddArg(true);
		ObjBool& using_param = (ObjBool&)**args.rbegin();

		auto itr = scripts.begin();

		Result result;
		bool result_set = false;

		for (; itr != scripts.end(); ++itr)
		{
			ScriptState& state = *itr->second;
			if (!state.FunctionAllowed(function)) continue;

			try
			{				
				bool found = false;
				Result r = Caller::Call(state, function, &found, DEFAULT_TIMEOUT);

				// The first result with any non-nil values is used
				if (found && !result_set) {
					for (size_t i = 0; i < r.size(); i++) {
						if (r[i].GetType() != TYPE_NIL) {
							result = r;
							result_set = true;

							// Change the 'using' argument so other scripts know
							// they won't be considered
							using_param = false;
							break;
						}
					}
				}
			} 
			catch (std::exception & e)
			{
				// script errored, process it (for now just rethrow)
				// todo: add handling
				HandleError(state, e);
				//throw;
			}
		}

		this->Clear();

		return result;
	}

	// Called when an error is raised by a script.
	void HandleError(ScriptState& state, std::exception & e)
	{
		// log error here

		
		// Process the callstack, block last Phasor -> Script function called
		// there should only be one.
		bool blocked = false;
		while (!state.callstack.empty())
		{
			Manager::ScriptCallstack& entry = *state.callstack.top();

			// write callstack entry to error file here

			if (!blocked && !entry.scriptInvoked) { 
				printf("Blocking function : %s\n", entry.func.c_str());
				state.BlockFunction(entry.func);
				blocked = true;
			}

			state.callstack.pop();
		}

		// log 

		
	}
}