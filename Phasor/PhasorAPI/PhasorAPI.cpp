#include "PhasorAPI.h"
#include "../Common/Common.h"
#include <list>

using namespace Common;
using namespace Manager;

namespace PhasorAPI
{
	void testf(CallHandler& handler, 
		Object::unique_deque& args, Object::unique_list& results)
	{
		ObjBool& b = (ObjBool&)*args[1];
		printf("Received %i value %i\n", args[1]->GetType(), b.GetValue());
		results.push_back(std::unique_ptr<Object>(
			new ObjString("Hello, register test.")));
	}

	// Functions to register for scripts use.
	// When any function is called all parameters have been type checked
	// and so can be treated as valid.
	const ScriptCallback PhasorExportTable[] =
	{
		// {&cfunc, "funcname", min_args, {arg1_t, arg2_t, .. argn_t}}
		// remember: n > min_args if there are overloads.
		{&testf, "test_func", 3, {TYPE_NUMBER, TYPE_BOOL, TYPE_STRING}}
	};

	void Register(Manager::ScriptState& state)
	{
		RegisterFunctions(state, PhasorExportTable, 
			sizeof(PhasorExportTable)/sizeof(Manager::ScriptCallback));
	}
}