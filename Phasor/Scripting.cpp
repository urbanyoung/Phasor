#include "Scripting.h"
#include "Manager.h"
#include "Common.h"
#include "PhasorAPI.h"
#include "PhasorScript.h"
#include <string>
#include <sstream>
#include <windows.h> // for GetCurrentProcessId()
#include <array>

std::unique_ptr<Scripting::Scripts> g_Scripts;

namespace Scripting
{
	using namespace Common;
	const std::wstring log_prefix = L" ---> ";

	#define DEFAULT_TIMEOUT 2000

	static const DWORD versions[] = {1234};

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


	// --------------------------------------------------------------------
	//
	Scripts::Scripts(COutStream& errstream, const std::string& scriptsDir)
		: errstream(errstream), scriptsDir(scriptsDir)
	{
	}

	void Scripts::OpenScript(const char* script)
	{
		// Make sure the script isn't already loaded
		if (scripts.find(script) != scripts.end()) {
			errstream << script << " is already loaded." << endl;
			return;
		}

		std::stringstream abs_file;
		abs_file << scriptsDir << "\\" << script << ".lua";

		try
		{
			std::string file = abs_file.str();
			std::unique_ptr<ScriptState> state = Manager::OpenScript(file.c_str());
			state->SetInfo(file, script);

			CheckScriptCompatibility(*state, script);
			PhasorAPI::Register(*state);

			// Notify the script that it's been loaded.
			bool fexists = false;
			Manager::Caller caller;
			caller.AddArg(GetCurrentProcessId());
			caller.Call(*state, "OnScriptLoad", &fexists, DEFAULT_TIMEOUT);

			if (!fexists) {
				HandleError(*state, "OnScriptLoad undefined.");
				return;
			}

			scripts[script] = std::move(state);
		}
		catch (std::exception& e)
		{
			NoFlush _(errstream);
			errstream << script << " cannot be loaded. Why? " <<
				endl << log_prefix << e.what() << endl;
		}
	}

	void Scripts::CloseScript(const char* script) 
	{
		auto itr = scripts.find(script);

		if (itr != scripts.end()) {
			std::unique_ptr<ScriptState> state(std::move(itr->second));
			scripts.erase(itr);
			
			try 
			{
				Manager::Caller caller;
				caller.Call(*state, "OnScriptUnload", DEFAULT_TIMEOUT);
			} catch (std::exception & e) {
				HandleError(*state, e.what());
			}
			//Manager::CloseScript(state);		
			// script closed when state goes out of scope
		}
	}

	void Scripts::CheckScriptCompatibility(ScriptState& state, const char* script) 
	{
		bool funcExists = false;
		Manager::Caller caller;
		Result result = caller.Call(state, "GetRequiredVersion", &funcExists, DEFAULT_TIMEOUT);

		if (!funcExists) {
			throw std::exception("GetRequiredVersion undefined.");
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
			throw std::exception("Not compatible with this version of Phasor.");
		}
	}

	// Called when an error is raised by a script.
	void Scripts::HandleError(ScriptState& state, const std::string& desc)
	{
		NoFlush _(errstream); // flush once at the end of the func

		errstream << L"Error in '" << state.GetName() << L'\'' <<  endl;
		errstream << log_prefix << L"Path: " << state.GetPath() << endl;
		errstream << log_prefix << L"Error: " << desc << endl;

		std::string blockedFunc;

		// Process the callstack, block last Phasor -> Script function called
		// there should only be one.
		bool blocked = false;
		while (!state.callstack.empty())
		{
			Manager::ScriptCallstack& entry = *state.callstack.top();

			if (!blocked && !entry.scriptInvoked) { 
				state.BlockFunction(entry.func);
				blocked = true;
				blockedFunc = entry.func;
			}

			state.callstack.pop();
		}

		if (blocked) {
			errstream << log_prefix << L"Action: Further calls to '" << blockedFunc << 
				L"' will be ignored." << endl;
		}

		errstream << endl;			
	}

	// --------------------------------------------------------------------
	// 
	Result PhasorCaller::Call(const std::string& function, Scripts& s)
	{
		// This is the argument which indicates if a scripts' return value is used.
		this->AddArg(true);
		ObjBool& using_param = (ObjBool&)**args.rbegin();

		auto itr = s.scripts.begin();

		Result result;
		bool result_set = false;

		for (; itr != s.scripts.end(); ++itr)
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
				s.HandleError(state, e.what());
				//throw;
			}
		}

		this->Clear();

		return result;
	}
}