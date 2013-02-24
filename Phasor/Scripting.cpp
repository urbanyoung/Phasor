#include "Scripting.h"
#include "Manager.h"
#include "Common/Common.h"
#include "Common/MyString.h"
#include "PhasorAPI/PhasorAPI.h"
#include "ScriptingEvents.h"
#include <string>
#include <sstream>
#include <windows.h> // for GetCurrentProcessId()
#include <array>

namespace scripting
{
	using namespace Common;
	const std::wstring log_prefix = L"  ";
	
	// Versions which should use the most up to date api
	static const DWORD versions[] = {10059};

	// --------------------------------------------------------------------
	bool CheckCompatibility(const DWORD* table, size_t n, DWORD version)
	{
		for (size_t i = 0; i < n; i++) {
			if (table[i] == version) return true;
		}
		return false;
	}

	// Checks if the specified version is compatible with the currne api
	static bool CompatibleWithCurrent(DWORD required) 
	{
		const int n = sizeof(versions)/sizeof(versions[0]);
		return CheckCompatibility(versions, n, required);	
	}

	// --------------------------------------------------------------------
	//
	Scripts::Scripts(COutStream& errstream, const std::wstring& scriptsDir)
		: errstream(errstream), scriptsDir(NarrowString(scriptsDir))
	{
	}

	Scripts::~Scripts()
	{
		CloseAllScripts();
	}

	bool Scripts::OpenScript(const char* script)
	{		
		// Make sure the script isn't already loaded
		if (scripts.find(script) != scripts.end()) {
			errstream << script << " is already loaded." << endl;
			return false;
		}

		std::stringstream abs_file;
		abs_file << scriptsDir << script << ".lua";

		try
		{
			std::string file = abs_file.str();
			std::unique_ptr<ScriptState> state_ = Manager::CreateScript();
			std::unique_ptr<PhasorScript> phasor_state(
				new PhasorScript(std::move(state_)));  
			
			// load api then file so Phasor's funcs don't override any script ones
			PhasorAPI::Register(*phasor_state->state);
			phasor_state->state->DoFile(file.c_str());

			phasor_state->SetInfo(file, script);
			CheckScriptCompatibility(*phasor_state->state, script);

			// Notify the script that it's been loaded.
			bool fexists = false;
			Manager::Caller caller;
			caller.AddArg(GetCurrentProcessId());
			caller.Call(*phasor_state->state, "OnScriptLoad", &fexists, DEFAULT_TIMEOUT);

			if (!fexists) {
				throw std::exception("function 'OnScriptLoad' undefined.");
			}

			// Blacklist all functions that aren't defined
			const std::string* events = events::GetEventTable();
			size_t count = events::GetEventTableElementCount();
			for (size_t x = 0; x < count; x++) {
				if (!phasor_state->state->HasFunction(events[x].c_str()))
					phasor_state->BlockFunction(events[x]);
			}

			scripts[script] = std::move(phasor_state);
		}
		catch (std::exception& e)
		{
			NoFlush _(errstream);
			errstream << L"script '" << script << "' cannot be loaded." <<
				endl << log_prefix << e.what() << endl;
			return false;
		}
		return true;
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

		Manager::ScriptState* man_state = static_cast<Manager::ScriptState*>(phasor_state->state.get());
		CheckedScriptReference::ScriptBeingClosed(man_state);
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

	void Scripts::ReloadScripts()
	{
		std::list<std::string> reload_scripts;
		for(auto itr = scripts.begin(); itr != scripts.end(); ++itr)
			reload_scripts.push_back(itr->first);

		CloseAllScripts();

		for (auto itr = reload_scripts.begin(); itr != reload_scripts.end(); ++itr)
			OpenScript(itr->c_str());
	}

	bool Scripts::ReloadScript(const std::string& script)
	{
		auto itr = scripts.find(script);
		if (itr == scripts.end()) return false;
		CloseScript(itr);
		OpenScript(script.c_str());
		return true;
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
	// 

	std::list<CheckedScriptReference*> CheckedScriptReference::make_list()
	{
		return std::list<CheckedScriptReference*>();
	}

	void CheckedScriptReference::ScriptBeingClosed(Manager::ScriptState* state)
	{
		for (auto itr = refed_list.begin(); itr != refed_list.end(); ++itr) {		
			if ((*itr)->state == state) (*itr)->valid = false;
		}
	}

	CheckedScriptReference::CheckedScriptReference(Manager::ScriptState* state)
		: state(state)
	{
		refed_list.push_back(this);
	}

	CheckedScriptReference::~CheckedScriptReference()
	{
		auto itr = refed_list.begin();
		while (itr != refed_list.end()) {
			if (*itr == this) {
				refed_list.erase(itr);
				break;
			}
			itr++;
		}
	}

	bool CheckedScriptReference::still_valid() const {
		return valid;
	}

	std::list<CheckedScriptReference*> CheckedScriptReference::refed_list = CheckedScriptReference::make_list();


	// --------------------------------------------------------------------
	// 
	Result PhasorCaller::Call(const std::string& function, 
		std::array<Common::obj_type, 5> expected_types, Scripts& s)
	{
		// This is the argument which indicates if a scripts' return value is used.
		this->AddArg(!ignore_ret);
		ObjBool& using_param = (ObjBool&)**args.rbegin();

		Result result;
		bool result_set = ignore_ret;

		for (auto itr = s.scripts.begin(); itr != s.scripts.end(); ++itr)
		{
			PhasorScript& phasor_state = *itr->second;

			if (!phasor_state.FunctionAllowed(function))
				continue;

			try
			{				
				Manager::ScriptState& state = *phasor_state.state;
				bool found = false;
				Result r = Caller::Call(state, function, &found, DEFAULT_TIMEOUT);
			
				// The first result matching the expected types is used
				if (r.size() && !result_set) {
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
				s.HandleError(phasor_state, e.what());
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