#pragma once
#ifndef _SCRIPTING_H
#define _SCRIPTING_H
#include "Manager.h"
#include "Common/Streams.h"

typedef unsigned long DWORD;

/*
 * Each script is uniquely identified by its name relative to the 
 * scripts folder. So like
 * Scripts/
 *   MyScript/
 *     dostuff.lua     
 *   hello.lua
 *   
 *   dostuff.lua's unique would be MyScript/dostuff
 *   hello.lua's unique id would be hello
 */
namespace Scripting
{
	class Scripts;
}

extern std::unique_ptr<Scripting::Scripts> g_Scripts;

namespace Scripting
{
	typedef Manager::Result Result;

	// --------------------------------------------------------------------
	// Only reason this is a class is to ensure the errstream is set.
	class Scripts
	{
	private:
		std::string scriptsDir;
		std::map<std::string, std::unique_ptr<ScriptState>> scripts;
		COutStream& errstream;

		// Checks if the script is compatible with this version of Phasor.
		void CheckScriptCompatibility(ScriptState& state, const char* script);

		// Called when an error is raised by a script.
		void HandleError(ScriptState& state, const std::string& desc);

	public:
		Scripts(COutStream& errstream, const std::string& scriptsDir);

		// Opens the script specified, relative to the scripts directory
		void OpenScript(const char* script);

		// Closes the specified script, if it exists.
		void CloseScript(const char* script);

		friend class PhasorCaller;
	};

	// --------------------------------------------------------------------
	// Class: PhasorCaller
	// Interface for invoking function calls on all loaded scripts.
	// 
	class PhasorCaller : public Manager::Caller
	{
	public:
		// Calls the specified function on all loaded scripts.
		Result Call(const std::string& function,
			Scripts& s=*g_Scripts);
	};

}

#endif