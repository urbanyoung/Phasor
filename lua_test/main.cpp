#include <iostream>
#include <conio.h>
#include "..\Phasor\Lua.h"

using namespace std;
using namespace Scripting::Lua;

const char* script = "var = 2";

void main()
{
	State* state = State::NewState();

	try
	{
		state->DoString(script);
	}
	catch (std::exception e)
	{
		cout << e.what() << endl;
	}

	cout << "Press any key to continue...";
	getch();

	state->Close();
}