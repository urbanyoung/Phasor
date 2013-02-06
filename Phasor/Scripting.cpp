#include "Scripting.h"
#include "Manager.h"
#include "Common/Common.h"
#include "Common/MyString.h"
#include "PhasorAPI/PhasorAPI.h"
#include <string>
#include <sstream>
#include <windows.h> // for GetCurrentProcessId()
#include <array>

/*! \todo
 * Add type checking to return values from scripts.
 */

namespace Scripting
{
	using namespace Common;
	const std::wstring log_prefix = L"  ";
	#define DEFAULT_TIMEOUT 2000
	// Versions which should use the most up to date api
	static const DWORD versions[] = {10059};
	// Versions which should use the deprecated api
	static const DWORD deprecated_versions[] = {10058};

	// --------------------------------------------------------------------
	bool CheckCompatibility(const DWORD* table, size_t n, DWORD version)
	{
		for (size_t i = 0; i < n; i++) {
			if (table[i] == version)
				return true;
		}
		return false;
	}

	// Checks if the specified version is compatible with the currne api
	static bool CompatibleWithCurrent(DWORD required) 
	{
		const int n = sizeof(versions)/sizeof(versions[0]);
		return CheckCompatibility(versions, n, required);	
	}

	// Check if the specified version is compatible with the deprecated api
	static bool CompatibleWithDeprecated(DWORD version)
	{
		const int n = sizeof(deprecated_versions)/sizeof(deprecated_versions[0]);
		return CheckCompatibility(deprecated_versions, n, version);
	}

	// --------------------------------------------------------------------
	//
	Scripts::Scripts(COutStream& errstream, const std::wstring& scriptsDir)
		: errstream(errstream), scriptsDir(NarrowString(scriptsDir))
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
		abs_file << scriptsDir << script << ".lua";

		try
		{
			std::string file = abs_file.str();
			std::unique_ptr<ScriptState> state_ = Manager::OpenScript(file.c_str());
			std::unique_ptr<PhasorScript> phasor_state(
				new PhasorScript(state_));  
			
			phasor_state->SetInfo(file, script);

			bool current_api = CheckScriptCompatibility(*phasor_state->state, script);
			PhasorAPI::Register(*phasor_state->state, current_api);

			// Notify the script that it's been loaded.
			bool fexists = false;
			Manager::Caller caller;
			caller.AddArg(GetCurrentProcessId());
			caller.Call(*phasor_state->state, "OnScriptLoad", &fexists, DEFAULT_TIMEOUT);

			if (!fexists) {
				throw std::exception("function 'OnScriptLoad' undefined.");
			}

			scripts[script] = std::move(phasor_state);
		}
		catch (std::exception& e)
		{
			NoFlush _(errstream);
			errstream << L"script '" << script << "' cannot be loaded." <<
				endl << log_prefix << e.what() << endl;
		}
	}

	Scripts::scripts_t::iterator Scripts::CloseScript(scripts_t::iterator itr)
	{
		if (itr == scripts.end()) return itr;
		std::unique_ptr<PhasorScript> phasor_state(std::move(itr->second));
		itr = scripts.erase(itr);

		try {
			Manager::Caller caller;
			caller.Call(*phasor_state->state, "OnScriptUnload", DEFAULT_TIMEOUT);
		} catch (std::exception & e) {
			HandleError(*phasor_state, e.what());
		}
		return itr;
	}

	void Scripts::CloseScript(const char* script) 
	{
		auto itr = scripts.find(script);
		CloseScript(itr);
	}

	void Scripts::CloseAllScripts()
	{
		auto itr = scripts.begin();
		while (itr != scripts.end())
			itr = CloseScript(itr);
	}

	// Returns true if compatible with current api, false if compatible with deprecated
	// or throws an exception otherwise.
	bool Scripts::CheckScriptCompatibility(ScriptState& state, const char* script) 
	{
		bool funcExists = false;
		Manager::Caller caller;
		Result result = caller.Call(state, "GetRequiredVersion", &funcExists, DEFAULT_TIMEOUT);

		if (!funcExists) {
			throw std::exception("function 'GetRequiredVersion' undefined.");
		}

		if (result.GetType(0) != TYPE_NUMBER) throw std::exception("function 'GetRequiredVersion' should return a number.");
		DWORD requiredVersion = (DWORD)result.ReadNumber().GetValue();

		if (CompatibleWithCurrent(requiredVersion)) return true;
		else if (CompatibleWithDeprecated(requiredVersion)) return false;
		else throw std::exception("Not compatible with this version of Phasor.");
	}

	// Called when an error is raised by a script.
	void Scripts::HandleError(PhasorScript& phasor_state, const std::string& desc)
	{
		NoFlush _(errstream); // flush once at the end of the func

		errstream << L"Error in '" << phasor_state.GetName() << L'\'' <<  endl;
		errstream << log_prefix << L"Path: " << phasor_state.GetPath() << endl;
		errstream << log_prefix << L"Error: " << desc << endl;

		std::string blockedFunc;

		// Process the callstack, block last Phasor -> Script function called
		// there should only be one.
		bool blocked = false;
		while (!phasor_state.state->callstack.empty())
		{
			Manager::ScriptCallstack& entry = *phasor_state.state->callstack.top();

			if (!blocked && !entry.scriptInvoked) { 
				phasor_state.BlockFunction(entry.func);
				blocked = true;
				blockedFunc = entry.func;
			}

			phasor_state.state->callstack.pop();
		}

		if (blocked) {
			errstream << log_prefix << L"Action: Further calls to '" << blockedFunc << 
				L"' will be ignored." << endl;
		}

		errstream << endl;			
	}

	// --------------------------------------------------------------------
	// 
	Result PhasorCaller::Call(const std::string& function, 
		std::array<Common::obj_type, 5> expected_types, Scripts& s)
	{
		// This is the argument which indicates if a scripts' return value is used.
		this->AddArg(true);
		ObjBool& using_param = (ObjBool&)**args.rbegin();

		auto itr = s.scripts.begin();

		Result result;
		bool result_set = false;

		for (; itr != s.scripts.end(); ++itr)
		{
			PhasorScript& phasor_state = *itr->second;
			if (!phasor_state.FunctionAllowed(function)) continue;

			try
			{				
				Manager::ScriptState& state = *phasor_state.state;
				bool found = false;
				Result r = Caller::Call(state, function, &found, DEFAULT_TIMEOUT);

				// The first result matching the expected types is used
				if (found && !result_set) {
					size_t nloop = expected_types.size() < r.size() ?
						expected_types.size() : r.size();
					bool use_result = true;
					Manager::MObject& obj = r.ReadObject();
					for (size_t i = 0; i < nloop; i++) {
						if (expected_types[i] != obj.GetType()) {
							std::unique_ptr<Manager::MObject> converted;
							if (!obj.ConvertTo(expected_types[i], &converted)) {
								use_result = false;
								break;
							} else r.Replace(i, std::move(converted));
						}
					}
					if (use_result) {
						result = r;
						result_set = true;
						// Change the 'using' argument so other scripts know
						// they won't be considered
						using_param = false;
					}
				}
			} 
			catch (std::exception & e)
			{
				// script errored, process it (for now just rethrow)
				// todo: add handling
				s.HandleError(phasor_state, e.what());
				//throw;
			}
		}

		this->Clear();

		return result;
	}

	// -------------------------------------------------------------------
	void PhasorScript::BlockFunction(const std::string& func)
	{
		blockedFunctions.insert(func);
	}

	void PhasorScript::SetInfo(const std::string& path, const std::string& name)
	{
		this->name = name;
		this->path = path;
	}

	const std::string& PhasorScript::GetName()
	{
		return name;
	}

	const std::string& PhasorScript::GetPath()
	{
		return path;
	}

	// Checks if the specified script function is allowed to be called.
	bool PhasorScript::FunctionAllowed(const std::string& func)
	{
		return blockedFunctions.find(func) == blockedFunctions.end();
	}
}