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
		CloseAllScripts(true);
	}

	bool Scripts::OpenScript(const char* script, bool persistent)
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
			std::unique_ptr<Manager::ScriptState> state_ = Manager::CreateScript();
			scripts[script] = std::unique_ptr<PhasorScript>(
				new PhasorScript(std::move(state_), persistent));  
			PhasorScript* phasor_state = scripts[script].get();
			
			// load api then file so Phasor's funcs don't override any script ones
			PhasorAPI::Register(*phasor_state->state);
			phasor_state->state->DoFile(file.c_str());

			phasor_state->SetInfo(file, script);
			CheckScriptCompatibility(*phasor_state->state, script);

			// Notify the script that it's been loaded.
			bool fexists = false;
			Manager::Caller caller;
			caller.AddArg(GetCurrentProcessId());
			caller.Call(*phasor_state->state, "OnScriptLoad", &fexists);

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
		}
		catch (std::exception& e)
		{
			auto itr = scripts.find(script);
			if (itr != scripts.end()) {
				CheckedScriptReference::ScriptBeingClosed(itr->second.get());
				scripts.erase(itr);
			}
			
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
			caller.Call(*phasor_state->state, "OnScriptUnload");
		} catch (std::exception & e) {
			HandleError(*phasor_state, e.what());
		}

		CheckedScriptReference::ScriptBeingClosed(phasor_state.get());
		return itr;
	}

	void Scripts::CloseScript(const char* script) 
	{
		auto itr = scripts.find(script);
		CloseScript(itr);
	}

	void Scripts::CloseAllScripts(bool include_persistent)
	{
		auto itr = scripts.begin();
		while (itr != scripts.end()) {
			if (include_persistent || !itr->second->isPersistent()) itr = CloseScript(itr);
			else itr++;
		}
	}

	void Scripts::ReloadScripts(bool include_persistent)
	{
		std::list<ScriptInfo> reload_scripts;
		for(auto itr = scripts.begin(); itr != scripts.end(); ++itr) {
			if (include_persistent || !itr->second->isPersistent()) {
				reload_scripts.push_back(ScriptInfo(itr->first, itr->second->isPersistent()));
			}
		}

		CloseAllScripts(include_persistent);

		for (auto itr = reload_scripts.begin(); itr != reload_scripts.end(); ++itr)
			OpenScript(itr->script.c_str(), itr->persistent);
	}

	bool Scripts::ReloadScript(const std::string& script)
	{
		auto itr = scripts.find(script);
		if (itr == scripts.end()) return false;
		bool persistent = itr->second->isPersistent();
		CloseScript(itr);
		OpenScript(script.c_str(), persistent);
		return true;
	}

	std::list<ScriptInfo> Scripts::getLoadedScripts() const
	{
		std::list<ScriptInfo> loaded;
		for (auto itr = scripts.cbegin(); itr != scripts.cend(); ++itr)
			loaded.push_back(ScriptInfo(itr->first, itr->second->isPersistent()));
		return loaded;
	}

	// Returns true if compatible with current api, false if compatible with deprecated
	// or throws an exception otherwise.
	bool Scripts::CheckScriptCompatibility(Manager::ScriptState& state, const char* script) 
	{
		bool funcExists = false;
		Manager::Caller caller;
		Result result = caller.Call(state, "GetRequiredVersion", &funcExists);

		if (!funcExists) {
			throw std::exception("function 'GetRequiredVersion' undefined.");
		}

		if (!result.size() || result.GetType(0) != TYPE_NUMBER) 
			throw std::exception("function 'GetRequiredVersion' should return a number.");

		DWORD requiredVersion = (DWORD)result.ReadNumber(0).GetValue();

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

	std::list<CheckedScriptReference*> CheckedScriptReference::refed_list = CheckedScriptReference::make_list();


	void CheckedScriptReference::ScriptBeingClosed(PhasorScript* state)
	{
		for (auto itr = refed_list.begin(); itr != refed_list.end(); ++itr) {		
			if ((*itr)->state == state) (*itr)->valid = false;
		}
	}

	CheckedScriptReference::CheckedScriptReference(Manager::ScriptState* state)
		: valid(true)
	{
		bool found = false;
		auto itr = g_Scripts->scripts.begin();
		while (itr != g_Scripts->scripts.end())	{
			if (itr->second->state.get() == state) {
				this->state = itr->second.get();
				found = true;
				break;
			}
			itr++;
		}
		if (!found) throw std::runtime_error("CheckedScriptReference unknown Manager::ScriptState");		
		refed_list.push_back(this);
	}

	CheckedScriptReference::~CheckedScriptReference()
	{
		auto itr = refed_list.begin();
		while (itr != refed_list.end()) {
			if (*itr == this) {
				itr = refed_list.erase(itr);
				break;
			}
			itr++;
		}
	}

	bool CheckedScriptReference::still_valid() const {
		return valid;
	}

	// --------------------------------------------------------------------
	// 
	bool PhasorCaller::Call(PhasorScript& phasor_state, 
		const std::string& function, const results_t& expected_types,
		Result& out_result)
	{
		if (!phasor_state.FunctionAllowed(function)) return false;

		try
		{				
			Manager::ScriptState& state = *phasor_state.state;
			bool found = false;
			out_result = Caller::Call(state, function, &found);
			// The first result matching the expected types is used
			if (!result_set && out_result.size()) {
				size_t nloop = expected_types.size() < out_result.size() ?
					expected_types.size() : out_result.size();

				bool use_result = true;
				
				for (size_t i = 0; i < nloop; i++) {
					Manager::MObject& obj = out_result.ReadObject(i);
					if (expected_types[i] != obj.GetType()) {
						std::unique_ptr<Manager::MObject> converted;
						if (!obj.ConvertTo(expected_types[i], &converted)) {
							out_result.Clear();
							use_result = false;
							break;
						} else out_result.Replace(i, std::move(converted));
					}
				}
				return use_result;
			}
		} 
		catch (std::exception & e)
		{
			scripts.HandleError(phasor_state, e.what());
			
		}
		return false;
	}

	Result PhasorCaller::Call(const std::string& function, 
		results_t expected_types)
	{
		result_set = false;
		// This is the argument which indicates if a scripts' return value is used.
		this->AddArg(!ignore_ret);
		ObjBool& using_param = (ObjBool&)**args.rbegin();

		Result result;
		bool result_set = ignore_ret;

		for (auto itr = scripts.scripts.begin(); itr != scripts.scripts.end(); ++itr)
		{
			Result r;
			if (Call(*itr->second, function, expected_types, 
				!result_set ? result : r))
			{
				result_set = true;
				using_param = true;
			}
		}

		result_set = false;
		this->Clear();
		return result;
	}

	Result PhasorCaller::Call(PhasorScript& script, const std::string& function, 
		results_t expected_types)
	{
		Result r;
		Call(script, function, expected_types, r);
		return r;
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