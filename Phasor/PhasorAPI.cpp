#include "PhasorAPI.h"
#include "Common.h"
#include <list>

using namespace Common;

namespace PhasorAPI
{
	void testf(std::deque<Object*>& args, std::list<Common::Object*>& results)
	{
		ObjBool* b = (ObjBool*)args[1];
		printf("Received %i value %i\n", args[1]->GetType(), b->GetValue());
		results.push_back(new ObjString("Hello, register test."));
	}

	const Manager::ScriptCallback PhasorExportTable[] =
	{
		// {&cfunc, "funcname", min_args, {arg1_t, arg2_t, .. argn_t}}
		// remember: n > min_args if there are overloads.
		{&testf, "test_func", 3, {TYPE_NUMBER, TYPE_BOOL, TYPE_STRING}}
	};

	void Register(ScriptState* state)
	{
		Manager::RegisterFunctions(state, PhasorExportTable, 
			sizeof(PhasorExportTable)/sizeof(Manager::ScriptCallback));
	}
}