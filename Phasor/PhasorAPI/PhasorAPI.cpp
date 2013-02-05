#include "PhasorAPI.h"
#include "../Common/Common.h"
#include <list>

#include "memory.h"

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
		{&testf, "test_func", 3, {TYPE_NUMBER, TYPE_BOOL, TYPE_STRING}},
		// Memory related functions: see memory.h
		{&l_readbit, "readbit", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_readbyte, "readbyte", 1, {TYPE_NUMBER, TYPE_NUMBER}},
		{&l_readchar, "readchar", 1, {TYPE_NUMBER}},
		{&l_readword, "readword", 1, {TYPE_NUMBER, TYPE_NUMBER}},
		{&l_readshort, "readshort", 1, {TYPE_NUMBER}},
		{&l_readdword, "readdword", 1, {TYPE_NUMBER, TYPE_NUMBER}},
		{&l_readint, "readint", 1, {TYPE_NUMBER}},
		{&l_readfloat, "readfloat", 1, {TYPE_NUMBER, TYPE_NUMBER}},
		{&l_readdouble, "readdouble", 1, {TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writebit, "writebit", 3, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writebyte, "writebyte", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writechar, "writechar", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writeword, "writeword", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writeshort, "writeshort", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writedword, "writedword", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writeint, "writeint", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writefloat, "writefloat", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writedouble, "writedouble", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}}
		// 
	};

	void Register(Manager::ScriptState& state)
	{
		RegisterFunctions(state, PhasorExportTable, 
			sizeof(PhasorExportTable)/sizeof(Manager::ScriptCallback));
	}
}