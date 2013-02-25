#include "PhasorAPI.h"
#include "../Common/Common.h"
#include <list>
#include <assert.h>
#include "memory.h"
#include "output.h"
#include "playerinfo.h"
#include "string.h"
#include "misc.h"
#include "misc_halo.h"
#include "haloobjects.h"
#include "alias_script.h"
#include "scripttimers.h"

using namespace Common;
using namespace Manager;

namespace PhasorAPI
{
	// Functions to register for scripts use.
	// When any function is called all parameters have been type checked
	// and so can be treated as valid.
	const ScriptCallback PhasorExportTable[] =
	{
		// {&cfunc, "funcname", min_args, {arg1_t, arg2_t, .. argn_t}}
		// remember: n > min_args if there are overloads.
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
		{&l_readstring, "readstring", 1, {TYPE_NUMBER, TYPE_NUMBER}},
		{&l_readwidestring, "readwidestring", 1, {TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writebit, "writebit", 3, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writebyte, "writebyte", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writechar, "writechar", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writeword, "writeword", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writeshort, "writeshort", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writedword, "writedword", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writeint, "writeint", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writefloat, "writefloat", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writedouble, "writedouble", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writestring, "writestring", 2, {TYPE_NUMBER, TYPE_STRING}},
		{&l_writewidestring, "writewidestring", 2, {TYPE_NUMBER, TYPE_STRING}},
		// Output related functions: see output.h
		{&l_hprintf, "hprintf", 1, {TYPE_STRING}},
		{&l_say, "say", 1, {TYPE_STRING}},
		{&l_privatesay, "privatesay", 2, {TYPE_NUMBER, TYPE_STRING}},
		{&l_sendconsoletext, "sendconsoletext", 2, {TYPE_NUMBER, TYPE_STRING}},
		{&l_respond, "respond", 1, {TYPE_STRING}},
		{&l_log_msg, "log_msg", 2, {TYPE_NUMBER, TYPE_STRING}},
		// Player info related functions: see playerinfo.h
		{&l_resolveplayer, "resolveplayer", 1, {TYPE_NUMBER}},
		{&l_rresolveplayer, "rresolveplayer", 1, {TYPE_NUMBER}},
		{&l_getplayer, "getplayer", 1, {TYPE_NUMBER}},
		{&l_getip, "getip", 1, {TYPE_NUMBER}},
		{&l_getport, "getport", 1, {TYPE_NUMBER}},
		{&l_getteam, "getteam", 1, {TYPE_NUMBER}},
		{&l_getname, "getname", 1, {TYPE_NUMBER}},
		{&l_gethash, "gethash", 1, {TYPE_NUMBER}},
		{&l_getteamsize, "getteamsize", 1, {TYPE_NUMBER}},
		{&l_getplayerobjectid, "getplayerobjectid", 1, {TYPE_NUMBER}},
		{&l_isadmin, "isadmin", 1, {TYPE_NUMBER}},
		{&l_setadmin, "setadmin", 1, {TYPE_NUMBER}},
		// String related functions: see string.h
		{&l_tokenizestring, "tokenizestring", 2, {TYPE_STRING, TYPE_STRING}},
		{&l_tokenizecmdstring, "tokenizecmdstring", 1, {TYPE_STRING}},
		// Miscellaneous functions: see misc.h
		{&l_getticks, "getticks", 0, {}},
		{&l_getrandomnumber, "getrandomnumber", 2, {TYPE_NUMBER, TYPE_NUMBER}},
		// Miscellaneous halo functions: see misc_halo.h
		{&l_changeteam, "changeteam", 2, {TYPE_NUMBER, TYPE_BOOL, TYPE_NUMBER}},
		{&l_kill, "kill", 1, {TYPE_NUMBER}},
		{&l_applycamo, "applycamo", 2, {TYPE_NUMBER, TYPE_NUMBER}},
		{&l_svcmd, "svcmd", 1, {TYPE_STRING, TYPE_BOOL}},
		{&l_updateammo, "updateammo", 1, {TYPE_NUMBER}},
		{&l_setammo, "setammo", 3, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_setspeed, "setspeed", 2, {TYPE_NUMBER, TYPE_NUMBER}},
		{&l_getprofilepath, "getprofilepath", 0, {}},
		// Object related halo functions: see haloobjects.h
		{&l_getobject, "getobject", 1, {TYPE_NUMBER}},
		{&l_getobjectcoords, "getobjectcoords", 1, {TYPE_NUMBER}},
		{&l_objecttoplayer, "objecttoplayer", 1, {TYPE_NUMBER}},
		{&l_createobject, "createobject", 7, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER, TYPE_BOOL, TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_destroyobject, "destroyobject", 1, {TYPE_NUMBER}},
		{&l_assignweapon, "assignweapon", 2, {TYPE_NUMBER, TYPE_NUMBER}},
		{&l_entervehicle, "entervehicle", 3, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_isinvehicle, "isinvehicle", 1, {TYPE_NUMBER}},
		{&l_exitvehicle, "exitvehicle", 1, {TYPE_NUMBER}},
		{&l_movobjectcoords, "movobjectcoords", 4, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_movobjectcoords, "moveobjectcoords", 4, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_gettagid, "gettagid", 2, {TYPE_STRING, TYPE_STRING}},
		{&l_gettagaddress, "gettagaddress", 1, {TYPE_NUMBER}},
		// Alias related functions: see alias_script.h
		{&l_alias_search, "alias_search", 2, {TYPE_STRING, TYPE_STRING}},
		{&l_alias_hash, "alias_hash", 2, {TYPE_STRING, TYPE_STRING}},
		// Timer related functions: see scripttimers.h
		{&l_registertimer, "registertimer", 2, {TYPE_NUMBER, TYPE_STRING, TYPE_ANY}},
		{&l_removetimer, "removetimer", 1, {TYPE_NUMBER}},

	};
	static const size_t export_table_size = sizeof(PhasorExportTable)/sizeof(PhasorExportTable[0]);

	void Register(Manager::ScriptState& state)
	{
		RegisterFunctions(state, PhasorExportTable, export_table_size);
	}
}