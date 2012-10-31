#pragma once

#include "Lua.h"

/*
 * Each script is uniquely identified by its name relative to the 
 * scripts folder. So like
 * Scripts/
 *   MyScript/
 *     dostuff.lua
 *     
 *   hello.lua
 *   
 *   dostuff.lua's unique would be MyScript/dostuff
 *   hello.lua's unique id would be hello
 */
namespace Scripting
{
	namespace Lua 
	{
		class Object;
	}

	// Sets the path to be used by this namespace (where scripts are).
	void SetPath(const char* scriptPath);

	/* Opens the script specified, relative to the scripts directory.
	 * May throw an exception <todo: add specific info>*/
	void OpenScript(const char* file);

	// Closes the specified script, if it exists.
	void CloseScript(const char* script);

	void Call(const char* function, const std::vector<Lua::Object*>& args);

}