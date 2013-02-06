#include "PhasorAPI.h"
#include "../Common/Common.h"
#include <list>
#include <assert.h>
#include "memory.h"
#include "output.h"
#include "deprecated.h"

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
		{&l_writedouble, "writedouble", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		// Output related functions: see output.h
		{&l_hprintf, "hprintf", 1, {TYPE_STRING}},
		{&l_sendconsoletext, "sendconsoletext", 2, {TYPE_NUMBER, TYPE_STRING}}
	};
	static const size_t export_table_size = sizeof(PhasorExportTable)/sizeof(PhasorExportTable[0]);

	// Deprecated functions (just the differences)
	ScriptCallback PhasorExportTableDeprecatedDiff[] =
	{
		// Memory related functions: see memory.h
		{&deprecated::l_writebit, "writebit", 4, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&deprecated::l_writebyte, "writebyte", 3, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&deprecated::l_writeword, "writeword", 3, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&deprecated::l_writedword, "writedword", 3, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&deprecated::l_writefloat, "writefloat", 3, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		// Output related functions: see output.h
		{&l_hprintf, "hprintf", 1, {TYPE_STRING, TYPE_NUMBER}},
	};
	// Keeps track of functions that have been removed. ie those in the deprecated
	// table but not in the current version's export table. Phasor will assert
	// if this value is incorrect.
	static const size_t n_removed_functions = 1;
	// Number of entries in the deprecated diff table.
	static const size_t n_deprecated_diff = sizeof(PhasorExportTableDeprecatedDiff)/sizeof(PhasorExportTableDeprecatedDiff[0]);

	// We build the deprecated table once at runtime
	ScriptCallback PhasorExportTableDeprecated[export_table_size + n_removed_functions];
	// Number of entries in the deprecated table.
	static const size_t n_deprecated_size = sizeof(PhasorExportTableDeprecated)/sizeof(PhasorExportTableDeprecated[0]);
	// We only want to build the table once.
	static bool bDeprecatedBuilt = false;

	void BuildDeprecatedTable()
	{
		// copy the default table over.
		for (size_t x = 0; x < export_table_size; x++)
			PhasorExportTableDeprecated[x] = PhasorExportTable[x];
		// overwrite each entry with the deprecated one, then set the
		// entry in deprecateddiff to null.
		for (size_t x = 0; x < n_deprecated_diff; x++) {
			for (size_t i = 0; i < export_table_size; i++) {
				if (!strcmp(PhasorExportTableDeprecated[i].name, PhasorExportTableDeprecatedDiff[x].name)) {
					// catch multiple entries in the table
					assert(PhasorExportTableDeprecatedDiff[x].func != NULL);
					PhasorExportTableDeprecated[i] = PhasorExportTableDeprecatedDiff[x];
					PhasorExportTableDeprecatedDiff[x].func = NULL;
					break;
				}
			}
		}
		// add the new entries (those in diff that aren't in PhasorExportTable)
		size_t i = export_table_size;
		for (size_t x = 0; x < n_deprecated_diff; x++) {
			if (PhasorExportTableDeprecatedDiff[x].func != NULL) {
				assert(i < n_deprecated_size);
				PhasorExportTableDeprecated[i++] = PhasorExportTableDeprecatedDiff[x];
			}
		}
		assert(i == n_deprecated_size);
		bDeprecatedBuilt = true;
	}

	void Register(Manager::ScriptState& state, bool deprecated)
	{
		size_t nentries = 0;
		const Manager::ScriptCallback* funcs;

		if (!deprecated) {
			nentries = export_table_size;
			funcs = PhasorExportTable;
		} else {
			if (!bDeprecatedBuilt) BuildDeprecatedTable();
			nentries = n_deprecated_size;
			funcs = PhasorExportTableDeprecated;
		}
		RegisterFunctions(state, funcs, nentries);
	}
}