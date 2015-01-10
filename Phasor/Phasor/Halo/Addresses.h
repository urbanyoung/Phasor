#pragma once

#ifdef PHASOR_PC
	#define CONST_MENTRY_SIZE				0x0A4 // size of each entry in map cycle
	#define GAMET_BUFFER_SIZE				0x098	
#elif PHASOR_CE
	#define CONST_MENTRY_SIZE				0x0E4
	#define GAMET_BUFFER_SIZE				0x0D8	
#endif
static_assert(GAMET_BUFFER_SIZE + 0x0C == CONST_MENTRY_SIZE, "incorrect mapcycle sizes");

// Offsets
#define OFFSET_CONSOLETEXT					0x0B4
#define OFFSET_RESPAWNTICKS					0x068
#ifdef PHASOR_PC
	#define MACHINE_ENTRY_SIZE				0x060
#elif PHASOR_CE
	#define MACHINE_ENTRY_SIZE				0x0EC
#endif

// This file is used to store all memory addresses Phasor uses
// ------------------------------------------------------------------------
// 
// Memory addresses
//
extern unsigned long ADDR_CONSOLEINFO;
extern unsigned long ADDR_RCONPLAYER;
extern unsigned long ADDR_TAGTABLE;
//extern unsigned long ADDR_PLAYERINFOBASE;
extern unsigned long ADDR_OBJECTBASE;
extern unsigned long ADDR_PLAYERBASE;
extern unsigned long ADDR_MAPCYCLEINDEX;
extern unsigned long ADDR_MAPCYCLELIST;
extern unsigned long ADDR_MAPCYCLECOUNT;
extern unsigned long ADDR_NEWGAMEMAP;
extern unsigned long ADDR_CURMAPCOUNT;
extern unsigned long ADDR_MAXMAPCOUNT;
extern unsigned long ADDR_SOCKETREADY;
extern unsigned long ADDR_GAMEREADY;
extern unsigned long ADDR_PREPAREGAME_FLAG; //updated (should work)
extern unsigned long ADDR_CMDCACHE;
extern unsigned long ADDR_CMDCACHE_INDEX;
extern unsigned long ADDR_CMDCACHE_CUR;
extern unsigned long ADDR_GAMETYPE;
extern unsigned long ADDR_PORT;
extern unsigned long ADDR_SERVERNAME;
extern unsigned long ADDR_CONSOLEREADY;
extern unsigned long ADDR_SERVERINFO;

// ------------------------------------------------------------------------
//
// Function addresses
//
extern unsigned long FUNC_HALOGETHASH;
extern unsigned long FUNC_EXECUTESVCMD; // fine.. i think
extern unsigned long FUNC_ONPLAYERDEATH;
extern unsigned long FUNC_ACTIONDEATH_1;
extern unsigned long FUNC_ACTIONDEATH_2;
extern unsigned long FUNC_ACTIONDEATH_3;
extern unsigned long FUNC_DOINVIS;
extern unsigned long FUNC_PLAYERJOINING; // fine.. i think
extern unsigned long FUNC_TEAMSELECT; // fine.. i think
extern unsigned long FUNC_GETMAPPATH;
extern unsigned long FUNC_VALIDATEGAMETYPE;
extern unsigned long FUNC_BUILDPACKET;
extern unsigned long FUNC_ADDPACKETTOQUEUE;
extern unsigned long FUNC_ADDPACKETTOPQUEUE;
extern unsigned long FUNC_AFTERSPAWNROUTINE; // fine.. i think
extern unsigned long FUNC_EXECUTEGAME;
extern unsigned long FUNC_PREPAREGAME_ONE;
extern unsigned long FUNC_PREPAREGAME_TWO;
extern unsigned long FUNC_BANPLAYER;
extern unsigned long FUNC_SAVEBANLIST;
extern unsigned long FUNC_UPDATEAMMO;
extern unsigned long FUNC_CREATEOBJECTQUERY;
extern unsigned long FUNC_CREATEOBJECT;
extern unsigned long FUNC_DESTROYOBJECT;
extern unsigned long FUNC_PLAYERASSIGNWEAPON;
extern unsigned long FUNC_NOTIFY_WEAPONPICKUP;
extern unsigned long FUNC_ENTERVEHICLE;
extern unsigned long FUNC_EJECTVEHICLE;
extern unsigned long FUNC_HALOEXCEPTIONHANDLER;

// ------------------------------------------------------------------------
//
// Codecave addresses
//
extern unsigned long CC_PHASORLOAD;
extern unsigned long CC_CONSOLEPROC;
extern unsigned long CC_CONSOLEHANDLER;
extern unsigned long CC_SERVERCMD;
extern unsigned long CC_GAMEEND;
extern unsigned long CC_PLAYERWELCOME;
extern unsigned long CC_CHAT;
extern unsigned long CC_MAPLOADING;
extern unsigned long CC_TEAMSELECTION;
extern unsigned long CC_NEWGAME;
extern unsigned long CC_PLAYERQUIT;
extern unsigned long CC_TEAMCHANGE;
extern unsigned long CC_DEATH;
extern unsigned long CC_KILLMULTIPLIER;
extern unsigned long CC_OBJECTINTERACTION;
extern unsigned long CC_PLAYERSPAWN;
extern unsigned long CC_PLAYERSPAWNEND;
extern unsigned long CC_VEHICLEENTRY;
extern unsigned long CC_WEAPONRELOAD;
extern unsigned long CC_DAMAGELOOKUP;
extern unsigned long CC_DAMAGEAPPLICATION;
extern unsigned long CC_WEAPONASSIGN;
extern unsigned long CC_OBJECTCREATION;
extern unsigned long CC_OBJECTDESTROY;
extern unsigned long CC_MAPCYCLEADD;
extern unsigned long CC_CLIENTUPDATE;
extern unsigned long CC_EXCEPTION_HANDLER;
extern unsigned long CC_VEHICLERESPAWN;
extern unsigned long CC_VEHICLEFORCEEJECT;
extern unsigned long CC_VEHICLEUSEREJECT;
extern unsigned long CC_HALOPRINT;
extern unsigned long CC_HALOBANCHECK;
extern unsigned long CC_PINGREQUEST;
extern unsigned long CC_EQUIPMENTDESTROY;

// ------------------------------------------------------------------------
//
// Patch addresses
//
extern unsigned long PATCH_ALLOCATEMAPNAME;
extern unsigned long PATCH_MAPTABLEALLOCATION;
extern unsigned long PATCH_MAPTABLE;
extern unsigned long PATCH_MAPLOADING;
extern unsigned long PATCH_NOMAPPROCESS;
extern unsigned long PATCH_TEAMSELECTION;

// TO FIND SIGNATURES FOR
extern unsigned long ADDR_BROADCASTVERSION; //
extern unsigned long ADDR_HASHLIST;
extern unsigned long ADDR_SERVERSTRUCT;
extern unsigned long CC_OBJECTCREATIONATTEMPT;
extern unsigned long ADDR_RCONPASSWORD;
extern unsigned long CC_SERVERCMDATTEMPT;
extern unsigned long PATCH_SERVERNAME1;
extern unsigned long PATCH_SERVERNAME2;
extern unsigned long PATCH_CURRENTVERSION;
extern unsigned long PATCH_ANYVERSIONCHECK1;
extern unsigned long PATCH_ANYVERSIONCHECK2;
extern unsigned long CC_HASHVALIDATE;
extern unsigned long CC_HASHVALIDATE_VALID;
extern unsigned long FUNC_VERIFYMAP_CE;
extern unsigned long FUNC_VEHICLERESPAWN1;
extern unsigned long FUNC_VEHICLERESPAWN2;
extern unsigned long CC_MACHINECONNECT;
extern unsigned long CC_MACHINEDISCONNECT;
extern unsigned long CC_MACHINEINFOFIX;
extern unsigned long FUNC_INTERSECT;

extern unsigned long CC_INTERSECT_RET1;
extern unsigned long CC_INTERSECT_RET2;
extern unsigned long CC_INTERSECT_RET3;
extern unsigned long CC_PROJMOVE;
extern unsigned long CC_PROJMOVE_RET1;
extern unsigned long CC_PROJMOVE_RET2;
extern unsigned long CC_ONTICKSLEEP;

//unsigned long ADDR_BROADCASTVERSION = 0x005df840;
//unsigned long ADDR_HASHLIST = 0x006A2AE4;
//unsigned long ADDR_SERVERSTRUCT = 0x00745BA0;
//unsigned long CC_OBJECTCREATIONATTEMPT = 0x52c600;
//unsigned long ADDR_RCONPASSWORD = 0x0069ba5c;
//unsigned long CC_SERVERCMDATTEMPT = 0x0051a26a;
//unsigned long PATCH_SERVERNAME1 = 0x00517d59;
//unsigned long PATCH_SERVERNAME2 = PATCH_SERVERNAME1 + 0x12;
//unsigned long PATCH_CURRENTVERSION =  0x005152e3;
//unsigned long PATCH_ANYVERSIONCHECK1 = PATCH_CURRENTVERSION + 4;
//unsigned long PATCH_ANYVERSIONCHECK2 = PATCH_ANYVERSIONCHECK1 + 0x0B;
//unsigned long CC_HASHVALIDATE = 0x0059BD97;
//unsigned long FUNC_VERIFYMAP_CE = 0x0048d980;
//unsigned long FUNC_VEHICLERESPAWN1 = 0x0052C310;
//unsigned long FUNC_VEHICLERESPAWN2 = 0x0052C2B0;


namespace Addresses
{
	// Called to find all the above addresses
	void LocateAddresses();
}