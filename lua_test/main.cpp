#include <vld.h> // check for memory leaks
#include <iostream>
#include <conio.h>
#include "..\Phasor\Scripting.h"
#include "..\Phasor\Streams.h"

const char* script = "i = 0\n"
	"function func(a)\n"
	"   print(a .. '\n')\n"
	"	while 1 do\n"
	"		i = i + 1\n"
	"		print(i)\n"
	"	end\n"
	"end\n";

void main()
{
	CLoggingStream errors(L"errorLogs");

		Scripting::SetErrorStream(&errors);
		Scripting::SetPath("D:\\Development\\C++\\Phasor - Copy\\Release");
		Scripting::OpenScript("lua_test");
		Scripting::OpenScript("lua_test1");
		//Scripting::OpenScript("lua_test1");

		Scripting::PhasorCaller caller;
		caller.AddArg("hello");
		Scripting::Result result = caller.Call("funca");
		caller.AddArg("hello_again");
		caller.Call("funca");
		std::cout << result.size() << std::endl;

	Scripting::CloseScript("lua_test");
	Scripting::CloseScript("lua_test1");
}