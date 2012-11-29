#pragma once
#ifndef _SCRIPTING_H
#define _SCRIPTING_H
#include "Manager.h"
#include "Streams.h"

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
	typedef Manager::Result Result;

	// --------------------------------------------------------------------
	// 

	// Sets the stream to report errors to
	// MUST BE SET BEFORE USING ANY OTHER FUNCTIONS
	void SetErrorStream(COutStream* errstream);
	
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
		Result Call(const std::string& function);
	};

}
#endif