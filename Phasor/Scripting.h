#pragma once

#include "Manager.h"

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

// todo: decide what to do when a script function errors (should it be blocked)
// todo: determine who should take responsibility for Scripting errors,
// Scripting interface should probably log the errors. Is caller responsible
// for catching the exceptions? I think yes.
namespace Scripting
{
	typedef Manager::Result Result;

	// --------------------------------------------------------------------
	// 
	
	// Sets the path to be used by this namespace (where scripts are).
	void SetPath(const char* scriptPath);

	// Opens the script specified, relative to the scripts directory
	// May throw an exception <todo: add specific info>
	void OpenScript(const char* script);

	// Closes the specified script, if it exists.
	// Guarantees the script is closed (if it exists) but may still throw
	// exception if an error occurs in OnScriptUnload
	void CloseScript(const char* script);

	// --------------------------------------------------------------------
	// Class: PhasorCaller
	// Interface for invoking function calls on all loaded scripts.
	// 
	class PhasorCaller : public Manager::Caller
	{
	public:
		// Calls the specified function on all loaded scripts.
		Result Call(const char* function);
	};

}