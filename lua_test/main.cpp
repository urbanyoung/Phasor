#include <vld.h> // check for memory leaks
#include <iostream>
#include <conio.h>
#include "..\Phasor\Scripting.h"
#include "..\Phasor\Streams.h"
#include "..\Phasor\PhasorThread.h"

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
	/*CLoggingStream errors(L"errorLogs");

	g_Scripts.reset(new Scripting::Scripts(errors, 
		"D:\\Development\\C++\\Phasor - Copy\\Release"));

	g_Scripts->OpenScript("lua_test");
	g_Scripts->OpenScript("lua_test1");
	//Scripting::OpenScript("lua_test1");

	Scripting::PhasorCaller caller;
	caller.AddArg("hello");
	Scripting::Result result = caller.Call("funca");
	caller.AddArg("hello_again");
	caller.Call("funca");
	std::cout << result.size() << std::endl;

	g_Scripts->CloseScript("lua_test");
	g_Scripts->CloseScript("lua_test1");*/
	PhasorThread thread;
	thread.run();
	std::unique_ptr<PhasorThreadEvent> e = TestEvent::Create(0);
	thread.InvokeInAux(std::move(e));
	int count = 0;
	while (1)
	{
		thread.ProcessEvents();
		Sleep(33);
		count++;
		if (count == 10) break;
	}
	thread.close();
	while (!thread.has_closed()) {
		Sleep(10);
	}
}