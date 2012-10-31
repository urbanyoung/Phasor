#include <vld.h> // check for memory leaks
#include <iostream>
#include <conio.h>
#include "..\Phasor\Lua.h"

using namespace std;
using namespace Scripting::Lua;

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
	State* state = State::NewState();

	try
	{
		//state->DoString(script);
		state->DoFile("D:\\Development\\C++\\Phasor\\Release\\lua_test.lua");
		vector<Object*> args;
		args.push_back(state->NewString("Hello"));
		state->Call("func", args, 5000);
	}
	catch (std::exception e)
	{
		cout << e.what() << endl;
	}

	State::Close(state);
}