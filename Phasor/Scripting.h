#pragma once

#include "Manager.h"
#include "Common/Streams.h"
#include "Common/noncopyable.h"
#include <array>
#include <set>
#include <string>

typedef unsigned long DWORD;
#define DEFAULT_TIMEOUT 2000

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

	public:
		std::unique_ptr<Manager::ScriptState> state;

		PhasorScript(std::unique_ptr<Manager::ScriptState>& state)
			: state(std::move(state)) {}
		virtual ~PhasorScript() {}

		void BlockFunction(const std::string& func);
		void SetInfo(const std::string& path, const std::string& name);

		const std::string& GetName();
		const std::string& GetPath();

		// Checks if the specified script function is allowed to be called.
		bool FunctionAllowed(const std::string& func);
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
		bool CheckScriptCompatibility(ScriptState& state, const char* script);

		// Called when an error is raised by a script.
		void HandleError(PhasorScript& state, const std::string& desc);

		scripts_t::iterator CloseScript(scripts_t::iterator itr);

	public:
		Scripts(COutStream& errstream, const std::wstring& scriptsDir);

		// Opens the script specified, relative to the scripts directory
		void OpenScript(const char* script);

		// Closes the specified script, if it exists.
		void CloseScript(const char* script);
		void CloseAllScripts();

		friend class PhasorCaller;
	};

	/*! \class CheckedScriptReference
	 *	\brief Inherit this class if you plan on storing any script states,
	 *	it can be used to determine if they're still valid.
	 *	
	 *	\todo check this actually works.
	 */
	class CheckedScriptReference
	{
	private:
		bool valid;

		static std::list<CheckedScriptReference*> make_list();
		static void ScriptBeingClosed(Manager::ScriptState* state);
		static std::list<CheckedScriptReference*> refed_list;

	protected:
		Manager::ScriptState* state;
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
		bool ignore_ret;
	public:
		PhasorCaller::PhasorCaller() : Manager::Caller(), ignore_ret(false)
		{
		}

		void ReturnValueIgnored() { ignore_ret = true; }
		
		// Calls the specified function on all loaded scripts.
		Result Call(const std::string& function, 
			results_t expected_types = results_t(),
			Scripts& s=*g_Scripts);
	};

}