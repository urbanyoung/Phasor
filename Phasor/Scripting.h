#pragma once

#include "Manager.h"
#include "Common/Streams.h"
#include "Common/noncopyable.h"
#include <array>
#include <set>
#include <string>

typedef unsigned long DWORD;

namespace scripting
{
	class Scripts;
}

extern std::unique_ptr<scripting::Scripts> g_Scripts;

namespace scripting
{
	typedef Manager::Result Result;

	class PhasorScript : noncopyable
	{
	private:
		std::set<std::string> blockedFunctions;
		std::string name, path;
		bool persistent;

	public:
		std::unique_ptr<Manager::ScriptState> state;

		PhasorScript(std::unique_ptr<Manager::ScriptState> state, bool persistent)
			: state(std::move(state)), persistent(persistent) {}
		virtual ~PhasorScript() {}

		void BlockFunction(const std::string& func);
		void SetInfo(const std::string& path, const std::string& name);

		const std::string& GetName();
		const std::string& GetPath();
		bool isPersistent() { return persistent; }

		// Checks if the specified script function is allowed to be called.
		bool FunctionAllowed(const std::string& func);
	};
	class CheckedScriptReference;
	
	struct ScriptInfo
	{
		std::string script;
		bool persistent;
		ScriptInfo(const std::string & script, bool persistent) 
			: script(script), persistent(persistent)
		{}
	};

	// --------------------------------------------------------------------
	// Only reason this is a class is to ensure the errstream is set.
	class Scripts
	{
	private:
		typedef std::map<std::string, std::unique_ptr<PhasorScript>> scripts_t; 
		std::string scriptsDir;
		scripts_t scripts;
		COutStream& errstream;

		// Checks if the script is compatible with this version of Phasor.
		bool CheckScriptCompatibility(Manager::ScriptState& state, const char* script);

		// Called when an error is raised by a script.
		void HandleError(PhasorScript& state, const std::string& desc);

		scripts_t::iterator CloseScript(scripts_t::iterator itr);

	public:
		Scripts(COutStream& errstream, const std::wstring& scriptsDir);
		~Scripts();

		void LoadPersistentScripts();

		// Opens the script specified, relative to the scripts directory
		bool OpenScript(const char* script, bool persistent);

		// Closes the specified script, if it exists.
		void CloseScript(const char* script);
		void CloseAllScripts(bool include_persistent);

		void ReloadScripts(bool include_persistent);
		bool ReloadScript(const std::string& script);

		// Returns a list of the loaded scripts
		std::list<ScriptInfo> getLoadedScripts() const;

		friend class PhasorCaller;
		friend class CheckedScriptReference;
	};

	/*! \class CheckedScriptReference
	 *	\brief Inherit this class if you plan on storing any script states,
	 *	it can be used to determine if they're still valid.
	 */
	class CheckedScriptReference
	{
	private:
		bool valid;

		static std::list<CheckedScriptReference*> make_list();
		static void ScriptBeingClosed(PhasorScript* state);
		static std::list<CheckedScriptReference*> refed_list;

	protected:
		PhasorScript* state;
	public:
		CheckedScriptReference(Manager::ScriptState* state);
		virtual ~CheckedScriptReference();
		bool still_valid() const;

		friend class Scripts;
	};

	// --------------------------------------------------------------------
	// Class: PhasorCaller
	// Interface for invoking function calls on all loaded scripts.
	// 
	typedef std::array<Common::obj_type, 5> results_t;
	class PhasorCaller : public Manager::Caller
	{
		bool ignore_ret, result_set;
		Scripts& scripts;

		bool Call(PhasorScript& phasor_state,
			const std::string& function, const results_t& expected_types,
			Result& out_result);

	public:
		PhasorCaller::PhasorCaller(Scripts& scripts=*g_Scripts) 
			: Manager::Caller(), scripts(scripts), ignore_ret(false),
			result_set(false) {}

		void ReturnValueIgnored() { ignore_ret = true; }
		
		// Calls the specified function on all loaded scripts.
		Result Call(const std::string& function, 
			results_t expected_types = results_t());
		// Calls the specified function on the specific script.
		Result Call(PhasorScript& script, const std::string& function, 
			results_t expected_types = results_t());
	};

}