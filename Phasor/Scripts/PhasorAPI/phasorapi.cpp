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
    std::vector<lua::callback::CFunc> funcTable
    {
    // Memory related functions: see memory.h
        {l_readbit, "readbit"},
        {l_readbyte, "readbyte"},
        {l_readchar, "readchar"},
        {l_readword, "readword"},
        {l_readshort, "readshort"},
        {l_readdword, "readdword"},
        {l_readint, "readint"},
        {l_readfloat, "readfloat"},
        {l_readdouble, "readdouble"},
        {l_readstring, "readstring"},
        {l_readwidestring, "readwidestring"},
        {l_writebit, "writebit"},
        {l_writebyte, "writebyte"},
        {l_writechar, "writechar"},
        {l_writeword, "writeword"},
        {l_writeshort, "writeshort"},
        {l_writedword, "writedword"},
        {l_writeint, "writeint"},
        {l_writefloat, "writefloat"},
        {l_writedouble, "writedouble"},
        {l_writestring, "writestring"},
        {l_writewidestring, "writewidestring"},
        // Output related functions: see output.h
        {l_hprintf, "hprintf"},
        {l_say, "say"},
        {l_privatesay, "privatesay"},
        {l_sendconsoletext, "sendconsoletext"},
        {l_respond, "respond"},
        {l_log_msg, "log_msg"},
        // Player info related functions: see playerinfo.h
        {l_resolveplayer, "resolveplayer"},
        {l_rresolveplayer, "rresolveplayer"},
        {l_getplayer, "getplayer"},
        {l_getip, "getip"},
        {l_getport, "getport"},
        {l_getteam, "getteam"},
        {l_getname, "getname"},
        {l_gethash, "gethash"},
        {l_getteamsize, "getteamsize"},
        {l_getplayerobject, "getplayerobject"},
        {l_getplayerobjectid, "getplayerobjectid"},
        {l_isadmin, "isadmin"},
        {l_getadminlvl, "getadminlvl"},
        {l_setadmin, "setadmin"},
        // String related functions: see string.h
        {l_tokenizestring, "tokenizestring"},
        {l_tokenizecmdstring, "tokenizecmdstring"},
        // Miscellaneous functions: see misc.h
        {l_getticks, "getticks"},
        {l_getrandomnumber, "getrandomnumber"},
        // Miscellaneous halo functions: see misc_halo.h
        {l_changeteam, "changeteam"},
        {l_kill, "kill"},
        {l_applycamo, "applycamo"},
        {l_svcmd, "svcmd"},
        {l_svcmdplayer, "svcmdplayer"},
        {l_updateammo, "updateammo"},
        {l_setammo, "setammo"},
        {l_setspeed, "setspeed"},
        {l_getprofilepath, "getprofilepath"},
        {l_getservername, "getservername"},
        // Object related halo functions: see haloobjects.h
        {l_getobject, "getobject"},
        {l_getobjectcoords, "getobjectcoords"},
        {l_objectaddrtoplayer, "objectaddrtoplayer"},
        {l_objectidtoplayer, "objectidtoplayer"},
        {l_createobject, "createobject"},
        {l_destroyobject, "destroyobject"},
        {l_assignweapon, "assignweapon"},
        {l_entervehicle, "entervehicle"},
        {l_isinvehicle, "isinvehicle"},
        {l_exitvehicle, "exitvehicle"},
        {l_movobjectcoords, "movobjectcoords"},
        {l_movobjectcoords, "moveobjectcoords"},
        {l_gettagid, "gettagid"},
        {l_gettaginfo, "gettaginfo"},
        {l_gettagaddress, "gettagaddress"},
        {l_halointersect, "halointersect"},
        // Alias related functions: see alias_script.h
        // {l_alias_search, "alias_search"},
        // {l_alias_hash, "alias_hash"},
        // Timer related functions: see scripttimers.h
        {l_registertimer, "registertimer"},
        {l_removetimer, "removetimer"},
        // Damage related functions: see damagelookup.h
        {l_odl_causer, "odl_causer"},
        {l_odl_receiver, "odl_receiver"},
        {l_odl_tag, "odl_tag"},
        {l_odl_multiplier, "odl_multiplier"},
        {l_odl_flags, "odl_flags"},
        // Damage application, see haloobjects.h
        {l_applydmg, "applydmg"},
        {l_applydmgtag, "applydmgtag"},
        // HTTP functions, see http.h
        {l_http, "http"},
        {l_httpraw, "httpraw"},
        // Script management functions
        //{l_raiseerror, "raiseerror"},
};
}