#include "phasorapi.h"
#include "damagelookup.h"
#include "haloobjects.h"
#include "memory.h"
#include "misc.h"
#include "misc_halo.h"
#include "output.h"
#include "playerinfo.h"
#include "scripttimers.h"
#include "string.h"
#include "http.h"

namespace phasorapi {
    std::vector<Func> funcTable
    {
    // Memory related functions: see memory.h
        {{l_readbit, "readbit"}, AccessMode::kWhileLoaded},
        {{l_readbyte, "readbyte"}, AccessMode::kWhileLoaded},
        {{l_readchar, "readchar"}, AccessMode::kWhileLoaded},
        {{l_readword, "readword"}, AccessMode::kWhileLoaded},
        {{l_readshort, "readshort"}, AccessMode::kWhileLoaded},
        {{l_readdword, "readdword"}, AccessMode::kWhileLoaded},
        {{l_readint, "readint"}, AccessMode::kWhileLoaded},
        {{l_readfloat, "readfloat"}, AccessMode::kWhileLoaded},
        {{l_readdouble, "readdouble"}, AccessMode::kWhileLoaded},
        {{l_readstring, "readstring"}, AccessMode::kWhileLoaded},
        {{l_readwidestring, "readwidestring"}, AccessMode::kWhileLoaded},
        {{l_writebit, "writebit"}, AccessMode::kWhileLoaded},
        {{l_writebyte, "writebyte"}, AccessMode::kWhileLoaded},
        {{l_writechar, "writechar"}, AccessMode::kWhileLoaded},
        {{l_writeword, "writeword"}, AccessMode::kWhileLoaded},
        {{l_writeshort, "writeshort"}, AccessMode::kWhileLoaded},
        {{l_writedword, "writedword"}, AccessMode::kWhileLoaded},
        {{l_writeint, "writeint"}, AccessMode::kWhileLoaded},
        {{l_writefloat, "writefloat"}, AccessMode::kWhileLoaded},
        {{l_writedouble, "writedouble"}, AccessMode::kWhileLoaded},
        {{l_writestring, "writestring"}, AccessMode::kWhileLoaded},
        {{l_writewidestring, "writewidestring"}, AccessMode::kWhileLoaded},
        {{l_findsig, "findsig"}, AccessMode::kWhileLoaded},
        // Output related functions: see output.h
        {{l_hprintf, "hprintf"}, AccessMode::kAlways},
        {{l_say, "say"}, AccessMode::kWhileLoaded},
        {{l_privatesay, "privatesay"}, AccessMode::kWhileLoaded},
        {{l_sendconsoletext, "sendconsoletext"}, AccessMode::kWhileLoaded},
        {{l_respond, "respond"}, AccessMode::kWhileLoaded},
        {{l_log_msg, "log_msg"}, AccessMode::kAlways},
        // Player info related functions: see playerinfo.h
        {{l_resolveplayer, "resolveplayer"}, AccessMode::kWhileLoaded},
        {{l_rresolveplayer, "rresolveplayer"}, AccessMode::kWhileLoaded},
        {{l_getplayer, "getplayer"}, AccessMode::kWhileLoaded},
        {{l_getip, "getip"}, AccessMode::kWhileLoaded},
        {{l_getport, "getport"}, AccessMode::kWhileLoaded},
        {{l_getteam, "getteam"}, AccessMode::kWhileLoaded},
        {{l_getname, "getname"}, AccessMode::kWhileLoaded},
        {{l_gethash, "gethash"}, AccessMode::kWhileLoaded},
        {{l_getteamsize, "getteamsize"}, AccessMode::kWhileLoaded},
        {{l_getplayerobject, "getplayerobject"}, AccessMode::kWhileLoaded},
        {{l_getplayerobjectid, "getplayerobjectid"}, AccessMode::kWhileLoaded},
        {{l_isadmin, "isadmin"}, AccessMode::kWhileLoaded},
        {{l_getadminlvl, "getadminlvl"}, AccessMode::kWhileLoaded},
        {{l_setadmin, "setadmin"}, AccessMode::kWhileLoaded},
        // String related functions: see string.h
        {{l_tokenizestring, "tokenizestring"}, AccessMode::kAlways},
        {{l_tokenizecmdstring, "tokenizecmdstring"}, AccessMode::kAlways},
        // Miscellaneous functions: see misc.h
        {{l_getticks, "getticks"}, AccessMode::kAlways},
        {{l_getrandomnumber, "getrandomnumber"}, AccessMode::kAlways},
        // Miscellaneous halo functions: see misc_halo.h
        {{l_changeteam, "changeteam"}, AccessMode::kWhileLoaded},
        {{l_kill, "kill"}, AccessMode::kWhileLoaded},
        {{l_applycamo, "applycamo"}, AccessMode::kWhileLoaded},
        {{l_svcmd, "svcmd"}, AccessMode::kWhileLoaded},
        {{l_svcmdplayer, "svcmdplayer"}, AccessMode::kWhileLoaded},
        {{l_updateammo, "updateammo"}, AccessMode::kWhileLoaded},
        {{l_setammo, "setammo"}, AccessMode::kWhileLoaded},
        {{l_setspeed, "setspeed"}, AccessMode::kWhileLoaded},
        {{l_getprofilepath, "getprofilepath"}, AccessMode::kAlways},
        {{l_getservername, "getservername"}, AccessMode::kAlways},
        // Object related halo functions: see haloobjects.h
        {{l_getobject, "getobject"}, AccessMode::kWhileLoaded},
        {{l_getobjectcoords, "getobjectcoords"}, AccessMode::kWhileLoaded},
        {{l_objectaddrtoplayer, "objectaddrtoplayer"}, AccessMode::kWhileLoaded},
        {{l_objectidtoplayer, "objectidtoplayer"}, AccessMode::kWhileLoaded},
        {{l_createobject, "createobject"}, AccessMode::kWhileLoaded},
        {{l_destroyobject, "destroyobject"}, AccessMode::kWhileLoaded},
        {{l_assignweapon, "assignweapon"}, AccessMode::kWhileLoaded},
        {{l_entervehicle, "entervehicle"}, AccessMode::kWhileLoaded},
        {{l_isinvehicle, "isinvehicle"}, AccessMode::kWhileLoaded},
        {{l_exitvehicle, "exitvehicle"}, AccessMode::kWhileLoaded},
        {{l_movobjectcoords, "movobjectcoords"}, AccessMode::kWhileLoaded},
        {{l_movobjectcoords, "moveobjectcoords"}, AccessMode::kWhileLoaded},
        {{l_gettagid, "gettagid"}, AccessMode::kWhileLoaded},
        {{l_gettaginfo, "gettaginfo"}, AccessMode::kWhileLoaded},
        {{l_gettagaddress, "gettagaddress"}, AccessMode::kWhileLoaded},
        {{l_halointersect, "halointersect"}, AccessMode::kWhileLoaded},
        // Alias related functions: see alias_script.h
        // {{l_alias_search, "alias_search"},
        // {{l_alias_hash, "alias_hash"},
        // Timer related functions: see scripttimers.h
        {{l_registertimer, "registertimer"}, AccessMode::kWhileLoaded},
        {{l_removetimer, "removetimer"}, AccessMode::kWhileLoaded},
        // Damage related functions: see damagelookup.h
        {{l_odl_causer, "odl_causer"}, AccessMode::kWhileLoaded},
        {{l_odl_receiver, "odl_receiver"}, AccessMode::kWhileLoaded},
        {{l_odl_tag, "odl_tag"}, AccessMode::kWhileLoaded},
        {{l_odl_multiplier, "odl_multiplier"}, AccessMode::kWhileLoaded},
        {{l_odl_flags, "odl_flags"}, AccessMode::kWhileLoaded},
        // Damage application, see haloobjects.h
        {{l_applydmg, "applydmg"}, AccessMode::kWhileLoaded},
        {{l_applydmgtag, "applydmgtag"}, AccessMode::kWhileLoaded},
        // HTTP functions, see http.h
        {{l_http, "http"}, AccessMode::kWhileLoaded}, // can't make new requests
        {{l_httpraw, "httpraw"}, AccessMode::kWhileLoaded},
        // Script management functions
        //{{l_raiseerror, "raiseerror"},
    };

    int l_func_inactive(lua_State* L)
    {
        luaL_error(L, "cannot use api function : unavailable after OnScriptUnload");
        return 0;
    }
}