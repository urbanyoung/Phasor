#include <iostream>
#include <conio.h>
#include "..\Phasor\Lua.h"

using namespace std;
using namespace Scripting::Lua;

const char* script = "i = 0\n"
	"function func()\n"
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
		state->DoString(script);
		state->Call("func", vector<Object*>(), 5000);
	}
	catch (std::exception e)
	{
		cout << e.what() << endl;
	}

	cout << "Press any key to continue...";
	getch();

	state->Close();
}