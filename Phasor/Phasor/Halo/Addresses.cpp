#include "../../Common/Common.h"
#include "../../Common/MyString.h"
#include "../Globals.h"

// This file is used to store all memory addresses Phasor uses
// ------------------------------------------------------------------------
// 
// Memory addresses
//
unsigned long ADDR_CONSOLEINFO = 0x0063BC1C;
unsigned long ADDR_RCONPLAYER = ADDR_CONSOLEINFO + 0x10;
unsigned long ADDR_TAGTABLE = 0x00460678;
unsigned long ADDR_PLAYERINFOBASE = 0x0069B91C;
unsigned long ADDR_OBJECTBASE = 0x00744C18;
unsigned long ADDR_PLAYERBASE = 0x0075ECE4;
unsigned long ADDR_MAPCYCLEINDEX = 0x00614B58;
unsigned long ADDR_MAPCYCLELIST = 0x00614B4C;
unsigned long ADDR_MAPCYCLECOUNT = 0x00614B50;
unsigned long ADDR_NEWGAMEMAP = 0x006713D8;
unsigned long ADDR_CURMAPCOUNT = 0x00692480;
unsigned long ADDR_MAXMAPCOUNT = 0x00692484;
unsigned long ADDR_SOCKETREADY = 0x0069B91C;
unsigned long ADDR_GAMEREADY = 0x00698DC8;
unsigned long ADDR_PREPAREGAME_FLAG = 0x00694528;
unsigned long ADDR_CMDCACHE = 0x0063FED4;
unsigned long ADDR_CMDCACHE_INDEX = 0x006406CE;
unsigned long ADDR_CMDCACHE_CUR = 0x006406D0;
unsigned long ADDR_GAMETYPE = 0x00671340;
unsigned long ADDR_PORT = 0x00625230;
unsigned long ADDR_SERVERNAME = 0x006265B0;
unsigned long ADDR_CONSOLEREADY = 0x0063BC28;
unsigned long ADDR_SERVERINFO = 0x00671420;

// ------------------------------------------------------------------------
//
// Function addresses
//
unsigned long FUNC_HALOGETHASH			= 0x0059BBB0;
unsigned long FUNC_EXECUTESVCMD			= 0x004EB7E0;
unsigned long FUNC_ONPLAYERDEATH		= 0x00490050;
unsigned long FUNC_ACTIONDEATH_1		= 0x00524410;
unsigned long FUNC_ACTIONDEATH_2		= 0x0057D510;
unsigned long FUNC_ACTIONDEATH_3		= 0x00495A10;
unsigned long FUNC_DOINVIS				= 0x0049AAA0;
unsigned long FUNC_PLAYERJOINING		= 0x00517290;
unsigned long FUNC_TEAMSELECT			= 0x00490940;
unsigned long FUNC_GETMAPPATH			= 0x0045FC20;
unsigned long FUNC_VALIDATEGAMETYPE		= 0x00481830;
unsigned long FUNC_BUILDPACKET			= 0x00522E30;
unsigned long FUNC_ADDPACKETTOQUEUE		= 0x00516610;
unsigned long FUNC_ADDPACKETTOPQUEUE	= 0x00516460;
unsigned long FUNC_AFTERSPAWNROUTINE	= 0x00498920;
unsigned long FUNC_EXECUTEGAME			= 0x0047F0E0;
unsigned long FUNC_PREPAREGAME_ONE		= 0x004ED240;
unsigned long FUNC_PREPAREGAME_TWO		= 0x005193F0;
unsigned long FUNC_BANPLAYER			= 0x00518890;
unsigned long FUNC_SAVEBANLIST			= 0x00518270;
unsigned long FUNC_UPDATEAMMO			= 0x004E83E0;
unsigned long FUNC_CREATEOBJECTQUERY	= 0x0052C4F0;
unsigned long FUNC_CREATEOBJECT			= 0x0052C600;
unsigned long FUNC_DESTROYOBJECT		= 0x0052CD20;
unsigned long FUNC_PLAYERASSIGNWEAPON	= 0x00582C60;
unsigned long FUNC_NOTIFY_WEAPONPICKUP	= 0x00499EF0;
unsigned long FUNC_ENTERVEHICLE			= 0x0049A2A0;
unsigned long FUNC_EJECTVEHICLE			= 0x00580B00;
unsigned long FUNC_HALOEXCEPTIONHANDLER = 0x005B036C;

// ------------------------------------------------------------------------
//
// Codecave addresses
//
unsigned long CC_PHASORLOAD = 0x00595A12;
unsigned long CC_CONSOLEPROC = 0x004EB325;
unsigned long CC_CONSOLEHANDLER = 0x004B9FF0;
unsigned long CC_SERVERCMD = FUNC_EXECUTESVCMD;
unsigned long CC_GAMEEND = 0x00486B80;
unsigned long CC_PLAYERWELCOME = 0x0051692B;
unsigned long CC_CHAT = 0x004CEBC7;
unsigned long CC_MAPLOADING = 0x00483376;
unsigned long CC_TEAMSELECTION = 0x00513BB4;
unsigned long CC_NEWGAME = 0x0047B3B0;
unsigned long CC_PLAYERQUIT = 0x00494780;
unsigned long CC_TEAMCHANGE = 0x00490AD2;
unsigned long CC_DEATH = 0x00480168;
unsigned long CC_KILLMULTIPLIER = 0x004800D9;
unsigned long CC_OBJECTINTERACTION = 0x0049977B;
unsigned long CC_PLAYERSPAWN = 0x0049909A;
unsigned long CC_PLAYERSPAWNEND = 0x004990DF;
unsigned long CC_VEHICLEENTRY = 0x0049A395;
unsigned long CC_WEAPONRELOAD = 0x004E8303;
unsigned long CC_DAMAGELOOKUP = 0x00524fd0;
unsigned long CC_DAMAGEAPPLICATION = 0x00525864;
unsigned long CC_WEAPONASSIGN = 0x005827AC;
//unsigned long CC_WEAPONCREATION = 0x004E5F3F;
//unsigned long CC_WEAPONCREATION = 0x0052CA29;
unsigned long CC_OBJECTCREATION = 0x0052CA1B; // all objects??
unsigned long CC_MAPCYCLEADD = 0x005191CE;
unsigned long CC_CLIENTUPDATE = 0x00578D6D;
unsigned long CC_EXCEPTION_HANDLER = 0x005B036C;
unsigned long CC_VEHICLERESPAWN = 0x00586D81;
unsigned long CC_EQUIPMENTDESTROY = 0x0047E61C;
unsigned long CC_VEHICLEFORCEEJECT = 0x0056E6CD;
unsigned long CC_VEHICLEUSEREJECT = 0x0056E107;
unsigned long CC_HALOPRINT = 0x004BA3F0;
unsigned long CC_HALOBANCHECK = 0x00518820;
unsigned long CC_PINGREQUEST = 0x5130F4; // ce 4c99e4

// ------------------------------------------------------------------------
//
// Patch addresses
//
unsigned long PATCH_ALLOCATEMAPNAME = 0x005191C4;
unsigned long PATCH_MAPTABLEALLOCATION = 0x004B82D1;
unsigned long PATCH_MAPTABLE = 0x0069247C;
unsigned long PATCH_MAPLOADING = 0x0047EF22;
unsigned long PATCH_NOMAPPROCESS = 0x00483280;
unsigned long PATCH_TEAMSELECTION = 0x00513BAE;

// TO FIND SIGNATURES FOR
unsigned long ADDR_BROADCASTVERSION = 0x005df840;
unsigned long ADDR_HASHLIST = 0x006A2AE4;
unsigned long ADDR_SERVERSTRUCT = 0x00745BA0;
unsigned long CC_OBJECTCREATIONATTEMPT = 0x52c600;
unsigned long ADDR_RCONPASSWORD = 0x0069ba5c;
unsigned long CC_SERVERCMDATTEMPT = 0x0051a26a;
unsigned long PATCH_SERVERNAME1 = 0x00517d59;
unsigned long PATCH_SERVERNAME2 = PATCH_SERVERNAME1 + 0x12;
unsigned long PATCH_CURRENTVERSION =  0x005152e3;
unsigned long PATCH_ANYVERSIONCHECK1 = PATCH_CURRENTVERSION + 4;
unsigned long PATCH_ANYVERSIONCHECK2 = PATCH_ANYVERSIONCHECK1 + 0x0B;
unsigned long CC_HASHVALIDATE = 0x0059BD97;
unsigned long CC_HASHVALIDATE_VALID = CC_HASHVALIDATE - 0x2f;
unsigned long FUNC_VERIFYMAP_CE = 0x0048d980;
unsigned long FUNC_VEHICLERESPAWN1 = 0x0052C310;
unsigned long FUNC_VEHICLERESPAWN2 = 0x0052C2B0;
unsigned long CC_MACHINECONNECT = 0x0051596c;
unsigned long CC_MACHINEDISCONNECT = 0x00515bd9;
unsigned long CC_MACHINEINFOFIX = 0x00516e39;
unsigned long FUNC_INTERSECT = 0x0053d8d0;
unsigned long CC_OBJECTDESTROY = 0x52f1e0;

unsigned long CC_INTERSECT_RET1 = 0x0053E00A;
unsigned long CC_INTERSECT_RET2 = 0x0053E051;
unsigned long CC_INTERSECT_RET3 = 0x0053E07D;
unsigned long CC_PROJMOVE = 0x004E2420;
unsigned long CC_PROJMOVE_RET1 = 0x004E32C0;
unsigned long CC_PROJMOVE_RET2 = 0x004E3363;
unsigned long CC_ONTICKSLEEP = 0x4ef264;

namespace Addresses
{
	using namespace Common;

	// ------------------------------------------------------------------------
	// 
	// Locates an address given a signature/offset
	// Note: If the address is not found, an exception is thrown.
	DWORD FindAddress(const char* desc, LPBYTE data, DWORD size, LPBYTE sig,
		DWORD sig_len, DWORD occ, int offset, LPBYTE wildcard=NULL)
	{
		DWORD dwAddress = 0, result = 0;
		bool success = FindSignature(sig, wildcard, sig_len, data, size, occ, result);
		if (success)
			dwAddress = (DWORD)data + result + offset;
		else {
			std::string err = m_sprintf("FindAddress - Signature not found for local function %s", desc);
			throw std::exception(err.c_str());
		}
		g_PrintStream->print("%-25s : %-25s %08X", __FUNCTION__, desc, dwAddress);
		return dwAddress;
	}

	// Locates an address given a signature/offset and reads 4 bytes of data
	// Note: If the address is not found, an exception is thrown.
	DWORD FindPtrAddress(const char* desc, LPBYTE data, DWORD size, LPBYTE sig, DWORD sig_len, DWORD occ, int offset)
	{
		DWORD dwAddress = FindAddress(desc, data, size, sig, sig_len, occ, offset);
		//DWORD dwAddress = (DWORD)data + result + offset;
		ReadBytes(dwAddress, &dwAddress, 4);
		g_PrintStream->print("%-25s : %-25s %08X", __FUNCTION__, desc, dwAddress);
		return dwAddress;
	}

	// Called to find all the above addresses
	void LocateAddresses()
	{
		// find the server's code section
		LPBYTE module = (LPBYTE)GetModuleHandle(0);
		DWORD OffsetToPE = *(DWORD*)(module + 0x3C);
		DWORD codeSize = *(DWORD*)(module + OffsetToPE + 0x1C);
		DWORD BaseOfCode = *(DWORD*)(module + OffsetToPE + 0x2C);
		LPBYTE codeSection = (LPBYTE)(module + BaseOfCode);

		// attempt to locate all addresses, if there are any errors an exception
		// will be thrown
		// ----------------------------------------------------------------
		BYTE sig1[] = {0xB1, 0x01, 0x83, 0xC4, 0x08, 0x8B, 0xF0};
		ADDR_CONSOLEINFO = FindPtrAddress("ADDR_CONSOLEINFO", codeSection, codeSize, sig1, sizeof(sig1), 0, 0x22);
		ADDR_RCONPLAYER = ADDR_CONSOLEINFO + 0x10;

		BYTE sig2[] = {0x51, 0x68, 0x00, 0x00, 0x00, 0x40};
		ADDR_TAGTABLE = FindPtrAddress("ADDR_TAGTABLE", codeSection, codeSize, sig2, sizeof(sig2), 0, -76);

		BYTE sig3[] = {0x05, 0x14, 0x0B, 0x00, 0x00};
		ADDR_PLAYERINFOBASE = FindPtrAddress("ADDR_PLAYERINFOBASE", codeSection, codeSize, sig3, sizeof(sig3), 0, 8);

		BYTE sig4[] = {0xDD, 0xD8, 0xF6, 0xC4, 0x41};
		ADDR_OBJECTBASE = FindPtrAddress("ADDR_OBJECTBASE", codeSection, codeSize, sig4, sizeof(sig4), 0, 0x20);

		BYTE sig5[] = {0x35, 0x72, 0x65, 0x74, 0x69};
		ADDR_PLAYERBASE = FindPtrAddress("ADDR_PLAYERBASE", codeSection, codeSize, sig5, sizeof(sig5), 0, -8);

		BYTE sig6[] = {0x57, 0x33, 0xFF, 0x3B, 0xC7};
		ADDR_MAPCYCLEINDEX = FindPtrAddress("ADDR_MAPCYCLEINDEX", codeSection, codeSize, sig6, sizeof(sig6), 0, 0x60);
		ADDR_MAPCYCLELIST = FindPtrAddress("ADDR_MAPCYCLELIST", codeSection, codeSize, sig6, sizeof(sig6), 0, 0x1C);
		ADDR_MAPCYCLECOUNT = FindPtrAddress("ADDR_CURRENTMAPDATACOUNT", codeSection, codeSize, sig6, sizeof(sig6), 0, 9);

		//BYTE sig7[] = {0x8B, 0x0C, 0xF0, 0x8D, 0x3C, 0xF0};
		//ADDR_MAPCYCLECOUNT = FindPtrAddress("ADDR_MAPCYCLECOUNT", codeSection, codeSize, sig7, sizeof(sig7), 0, -24);
		//ADDR_MAPCYCLEDATA = FindPtrAddress("ADDR_MAPCYCLECOUNT", codeSection, codeSize, sig7, sizeof(sig7), 0, -4);

		BYTE sig8[] = {0x90, 0x8B, 0xC3, 0x25, 0xFF, 0xFF, 0x00, 0x00};
		ADDR_NEWGAMEMAP = FindPtrAddress("ADDR_NEWGAMEMAP", codeSection, codeSize, sig8, sizeof(sig8), 0, 0x25);

#ifdef PHASOR_PC
		BYTE sig9[] = {0x83, 0xC8, 0xFF, 0x5E, 0x81, 0xC4, 0x04, 0x01, 0x00, 0x00};
		ADDR_CURMAPCOUNT = FindPtrAddress("ADDR_CURMAPCOUNT", codeSection, codeSize, sig9, sizeof(sig9), 0, -88);
		ADDR_MAXMAPCOUNT = ADDR_CURMAPCOUNT + 4;
#elif PHASOR_CE
		BYTE sig9[] = {0x83, 0xF8, 0xFF, 0x74, 0x30, 0x85, 0xC0, 0x7C, 0x2C};
		ADDR_CURMAPCOUNT = FindPtrAddress("ADDR_CURMAPCOUNT", codeSection, codeSize, sig9, sizeof(sig9), 0, 9);
		ADDR_MAXMAPCOUNT = ADDR_CURMAPCOUNT + 4;
#endif

		BYTE sig10[] = {0xB8, 0xD3, 0x4D, 0x62, 0x10};
		ADDR_SOCKETREADY = FindPtrAddress("ADDR_SOCKETREADY", codeSection, codeSize, sig10, sizeof(sig10), 0, 0xA1);

		BYTE sig11[] = {0x8A, 0x83, 0x8A, 0x02, 0x00, 0x00, 0x84, 0xC0};
		ADDR_GAMEREADY = FindPtrAddress("ADDR_GAMEREADY", codeSection, codeSize, sig11, sizeof(sig11), 0, 0x4C);

		BYTE sig12[] = {0x56, 0x57, 0xB3, 0x01};
		ADDR_PREPAREGAME_FLAG = FindPtrAddress("ADDR_PREPAREGAME_FLAG", codeSection, codeSize, sig12, sizeof(sig12), 0, 0x1F);

		BYTE sig13[] = {0x8A, 0x07, 0x81, 0xEC, 0x00, 0x05, 0x00, 0x00};
		ADDR_CMDCACHE = FindPtrAddress("ADDR_CMDCACHE", codeSection, codeSize, sig13, sizeof(sig13), 0, 0x78);
		ADDR_CMDCACHE_INDEX = FindPtrAddress("ADDR_CMDCACHE_INDEX", codeSection, codeSize, sig13, sizeof(sig13), 0, 0x55);
		#ifdef PHASOR_PC
		ADDR_CMDCACHE_CUR = FindPtrAddress("ADDR_CMDCACHE_CUR", codeSection, codeSize, sig13, sizeof(sig13), 0, 0xB1);
		#elif PHASOR_CE
		ADDR_CMDCACHE_CUR = FindPtrAddress("ADDR_CMDCACHE_CUR", codeSection, codeSize, sig13, sizeof(sig13), 0, 0xB2);
		#endif
			
		FUNC_EXECUTESVCMD = FindAddress("FUNC_EXECUTESVCMD", codeSection, codeSize, sig13, sizeof(sig13), 0, 0);
		CC_SERVERCMD = FUNC_EXECUTESVCMD;

		BYTE sig14[] = {0xC3, 0x68, 0x00, 0x01, 0x00, 0x00};
		ADDR_GAMETYPE = FindPtrAddress("ADDR_GAMETYPE", codeSection, codeSize, sig14, sizeof(sig14), 1, 0x0C);

		BYTE sig15[] = {0xC1, 0xE6, 0x10, 0x25, 0x00, 0xFF, 0x00, 0x00};
		ADDR_PORT = FindPtrAddress("ADDR_PORT", codeSection, codeSize, sig15, sizeof(sig15), 0, 0x26);

		BYTE sig16[] = {0x6A, 0x3F, 0x8D, 0x44, 0x24, 0x10, 0x50};
		ADDR_SERVERNAME = FindPtrAddress("ADDR_SERVERNAME", codeSection, codeSize, sig16, sizeof(sig16), 0, 8);

		BYTE sig17[] = {0x33, 0xC9, 0x83, 0xF8, 0xFF, 0x0F, 0x95, 0xC1};
		ADDR_CONSOLEREADY = FindPtrAddress("ADDR_CONSOLEREADY", codeSection, codeSize, sig17, sizeof(sig17), 0, 0xC9);

		BYTE sig18[] = {0x8B, 0x4C, 0x24, 0x0C, 0x8B, 0x10, 0x39, 0x0A};
		FUNC_HALOGETHASH = FindAddress("FUNC_HALOGETHASH", codeSection, codeSize, sig18, sizeof(sig18), 0, -60);

		BYTE sig19[] = {0x66, 0xFF, 0x80, 0xAE, 0x00, 0x00, 0x00};
#ifdef PHASOR_PC
		FUNC_ONPLAYERDEATH = FindAddress("FUNC_ONPLAYERDEATH", codeSection, codeSize, sig19, sizeof(sig19), 0, -87);
#elif PHASOR_CE
		FUNC_ONPLAYERDEATH = FindAddress("FUNC_ONPLAYERDEATH", codeSection, codeSize, sig19, sizeof(sig19), 0, -90);
#endif
		BYTE sig20[] = {0xC7, 0x81, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F};
		FUNC_ACTIONDEATH_1 = FindAddress("FUNC_ACTIONDEATH_1", codeSection, codeSize, sig20, sizeof(sig20), 0, 0x13);

		BYTE sig21[] = {0xC7, 0x42, 0x04, 0x03, 0x00, 0x00, 0x00, 0x5F};
		FUNC_ACTIONDEATH_2 = FindAddress("FUNC_ACTIONDEATH_2", codeSection, codeSize, sig21, sizeof(sig21), 0, 0x0F);

		BYTE sig22[] = {0x8B, 0x41, 0x34, 0x83, 0xEC, 0x10, 0x53, 0x55, 0x56};
		FUNC_ACTIONDEATH_3 = FindAddress("FUNC_ACTIONDEATH_3", codeSection, codeSize, sig22, sizeof(sig22), 0, -6);
			
		BYTE sig23[] = {0x83, 0xFB, 0xFF, 0x55, 0x8B, 0x6C, 0x24, 0x08};
		FUNC_DOINVIS = FindAddress("FUNC_DOINVIS", codeSection, codeSize, sig23, sizeof(sig23), 0, 0);

		BYTE sig24[] = {0x66, 0x8B, 0x46, 0x04, 0x83, 0xEC, 0x0C, 0x66, 0x85, 0xC0, 0x53};
		FUNC_PLAYERJOINING = FindAddress("FUNC_PLAYERJOINING", codeSection, codeSize, sig24, sizeof(sig24), 0, 0);

		BYTE sig25[] = {0x83, 0xEC, 0x10, 0x32, 0xC0, 0x85, 0xC9};
		FUNC_TEAMSELECT = FindAddress("FUNC_TEAMSELECT", codeSection, codeSize, sig25, sizeof(sig25), 0, -6);

		BYTE sig26[] = {0xC6, 0x00, 0x01, 0x5E};
		FUNC_GETMAPPATH = FindAddress("FUNC_GETMAPPATH", codeSection, codeSize, sig26, sizeof(sig26), 0, 0x0E);

#ifdef PHASOR_PC
		BYTE sig27[] =    {0x81, 0xEC, 0x60, 0x02, 0x00, 0x00, 0x53, 0x55, 0x8B, 0xAC, 0x24, 0x6C, 0x02, 0x00, 0x00, 0x56, 0x57, 0x8B, 0xD9};
		BYTE sig27_wc[] = {0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00};
		FUNC_VALIDATEGAMETYPE = FindAddress("FUNC_VALIDATEGAMETYPE", codeSection, codeSize, sig27, sizeof(sig27), 0, 0, sig27_wc);
#elif PHASOR_CE
		BYTE sig27[] =    {0x81, 0xEC, 0xA0, 0x02, 0x00, 0x00};
		FUNC_VALIDATEGAMETYPE = FindAddress("FUNC_VALIDATEGAMETYPE", codeSection, codeSize, sig27, sizeof(sig27), 0, 0);
#endif
		BYTE sig28[] = {0x81, 0xEC, 0xA4, 0x00, 0x00, 0x00, 0x53, 0x55};
		#ifdef PHASOR_PC
			FUNC_BUILDPACKET = FindAddress("FUNC_BUILDPACKET", codeSection, codeSize, sig28, sizeof(sig28), 0, 0);
		#elif PHASOR_CE
			FUNC_BUILDPACKET = FindAddress("FUNC_BUILDPACKET", codeSection, codeSize, sig28, sizeof(sig28), 1, 0);
		#endif

		BYTE sig29[] = {0x66, 0x8B, 0x46, 0x0E, 0x8A, 0xD0, 0xD0, 0xEA, 0xF6, 0xC2, 0x01};
		FUNC_ADDPACKETTOQUEUE = FindAddress("FUNC_ADDPACKETTOQUEUE", codeSection, codeSize, sig29, sizeof(sig29), 0, -56);

		BYTE sig30[] = {0x51, 0x53, 0x57, 0x8B, 0xF8, 0x32, 0xC0, 0x33, 0xC9};
		FUNC_ADDPACKETTOPQUEUE = FindAddress("FUNC_ADDPACKETTOPQUEUE", codeSection, codeSize, sig30, sizeof(sig30), 0, 0);

		BYTE sig31[] = {0x83, 0xEC, 0x2C, 0x53, 0x55};
		FUNC_AFTERSPAWNROUTINE = FindAddress("FUNC_AFTERSPAWNROUTINE", codeSection, codeSize, sig31, sizeof(sig31), 0, -9);

		// for this function we need to locate a call to FUNC_EXECUTEGAME and find the address from there
		BYTE sig32[] = {0x68, 0xCD, 0xCC, 0x4C, 0x3E};
#ifdef PHASOR_PC
		DWORD call_addr = FindAddress("FUNC_EXECUTEGAME", codeSection, codeSize, sig32, sizeof(sig32), 0, 0x9B);
#elif PHASOR_CE
		DWORD call_addr = FindAddress("FUNC_EXECUTEGAME", codeSection, codeSize, sig32, sizeof(sig32), 0, 0x99);
#endif
		DWORD call_offset = *(DWORD*)(call_addr + 1);
		FUNC_EXECUTEGAME = call_addr + 5 + call_offset;
		printf("extern unsigned long FUNC_EXECUTEGAME = 0x%08X", FUNC_EXECUTEGAME);
			
		BYTE sig33[] = {0x56, 0x68, 0xFF, 0x00, 0x00, 0x00, 0x57};
		FUNC_PREPAREGAME_ONE = FindAddress("FUNC_PREPAREGAME_ONE", codeSection, codeSize, sig33, sizeof(sig33), 0, 0);

		BYTE sig34[] = {0x83, 0xC4, 0x14, 0x8B, 0x44, 0x24, 04};
		FUNC_PREPAREGAME_TWO = FindAddress("FUNC_PREPAREGAME_TWO", codeSection, codeSize, sig34, sizeof(sig34), 0, -52);

		BYTE sig35[] = {0x53, 0x55, 0x8B, 0x6C, 0x24, 0x10, 0x57, 0x55};
		FUNC_BANPLAYER = FindAddress("FUNC_BANPLAYER", codeSection, codeSize, sig35, sizeof(sig35), 0, 0);

		BYTE sig36[] = {0x83, 0xEC, 0x6C, 0x53, 0x57};
		FUNC_SAVEBANLIST = FindAddress("FUNC_SAVEBANLIST", codeSection, codeSize, sig36, sizeof(sig36), 0, 0);

		BYTE sig37[] = {0x6A, 0x2D, 0x6A, 0x00, 0xBA, 0xF8, 0x7F, 0x00, 0x00};
		FUNC_UPDATEAMMO = FindAddress("FUNC_UPDATEAMMO", codeSection, codeSize, sig37, sizeof(sig37), 0, -117);

			
		BYTE sig38[] = {0x8D, 0xA4, 0x24, 0x00, 0x00, 0x00, 0x00, 0x8D, 0x64, 0x24, 0x00, 0x0F, 0xBF, 0xC5};
		CC_CONSOLEPROC = FindAddress("CC_CONSOLEPROC", codeSection, codeSize, sig38, sizeof(sig38), 0, -96);

		BYTE sig39[] =    {0xB8, 0x01, 0x00, 0x00, 0x00, 0xC6, 0x05, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
		BYTE sig39_wc[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00};
		CC_CONSOLEHANDLER = FindAddress("CC_CONSOLEHANDLER", codeSection, codeSize, sig39, sizeof(sig39), 0, 0, sig39_wc);
			
		BYTE sig40[] = {0x83, 0xEC, 0x0C, 0x6A, 0x00, 0x6A, 0x01, 0x6A, 0x00};
		CC_GAMEEND = FindAddress("CC_GAMEEND", codeSection, codeSize, sig40, sizeof(sig40), 0, 0);

		BYTE sig41[] = {0x0F, 0xB6, 0x4C, 0x24, 0x10, 0x49, 0x83, 0xF9, 0x24};
		#ifdef PHASOR_PC
			CC_PLAYERWELCOME = FindAddress("CC_PLAYERWELCOME", codeSection, codeSize, sig41, sizeof(sig41), 0, 0xCF);
		#elif PHASOR_CE
			CC_PLAYERWELCOME = FindAddress("CC_PLAYERWELCOME", codeSection, codeSize, sig41, sizeof(sig41), 0, 0xC1);	
		#endif

		BYTE sig42[] = {0x57, 0x33, 0xFF, 0x3B, 0xD7};
		CC_CHAT = FindAddress("CC_CHAT", codeSection, codeSize, sig42, sizeof(sig42), 0, 0x2D);

		BYTE sig43[] = {0x8B, 0x0C, 0x06, 0x6A, 0x3F, 0x51};
		CC_MAPLOADING = FindAddress("CC_MAPLOADING", codeSection, codeSize, sig43, sizeof(sig43), 0, 0);

		BYTE sig44[] = {0x80, 0x7B, 0x1E, 0xFF, 0xC7, 0x44, 0x24, 0x08, 0x25, 0x00, 0x00, 0x00, 0xC7, 0x44, 0x24, 0x0C, 0x7C, 0x00, 0x00, 0x00};
		CC_TEAMSELECTION = FindAddress("CC_TEAMSELECTION", codeSection, codeSize, sig44, sizeof(sig44), 0, 0x1A);
			
		BYTE sig45[] = {0x32, 0xC9, 0x83, 0xF8, 0x13};
		CC_NEWGAME = FindAddress("CC_NEWGAME", codeSection, codeSize, sig45, sizeof(sig45), 0, -26);

		BYTE sig46[] = {0x5F, 0xC1, 0xE1, 0x04, 0x48, 0x5E};
		CC_PLAYERQUIT = FindAddress("CC_PLAYERQUIT", codeSection, codeSize, sig46, sizeof(sig46), 0, -130);

		BYTE sig47[] = {0x8A, 0x04, 0x24, 0x3C, 0x10, 0x53, 0x0F, 0xB6, 0x5C, 0x24, 0x05};
		#ifdef PHASOR_PC
			CC_TEAMCHANGE = FindAddress("CC_TEAMCHANGE", codeSection, codeSize, sig47, sizeof(sig47), 0, 0x6E);
		#elif PHASOR_CE
			CC_TEAMCHANGE = FindAddress("CC_TEAMCHANGE", codeSection, codeSize, sig47, sizeof(sig47), 0, 0x6F);
		#endif

		BYTE sig48[] = {0x55, 0x8B, 0x6C, 0x24, 0x20, 0x89, 0x44, 0x24, 0x04};
		CC_DEATH = FindAddress("CC_DEATH", codeSection, codeSize, sig48, sizeof(sig48), 0, 0);

		BYTE sig49[] = {0x83, 0xEC, 0x10, 0x55, 0x8B, 0x6C, 0x24, 0x1C};
		CC_KILLMULTIPLIER = FindAddress("CC_KILLMULTIPLIER", codeSection, codeSize, sig49, sizeof(sig49), 0, 0x19);

		BYTE sig50[] = {0x53, 0x8B, 0x5C, 0x24, 0x20, 0x55, 0x8B, 0x6C, 0x24, 0x20, 0x81, 0xE5, 0xFF, 0xFF, 0x00, 0x00, 0xC1, 0xE5, 0x09};
		CC_OBJECTINTERACTION = FindAddress("CC_OBJECTINTERACTION", codeSection, codeSize, sig50, sizeof(sig50), 0, 0);
			
		BYTE sig51[] = {0x8B, 0x54, 0x24, 0x14, 0x6A, 0xFF};
		CC_PLAYERSPAWN = FindAddress("CC_PLAYERSPAWN", codeSection, codeSize, sig51, sizeof(sig51), 0, -52);
		CC_PLAYERSPAWNEND = FindAddress("CC_PLAYERSPAWNEND", codeSection, codeSize, sig51, sizeof(sig51), 0, 0x11);

		BYTE sig52[] = {0x33, 0xD2, 0x8A, 0x56, 0x64, 0x33, 0xC0};
		CC_VEHICLEENTRY = FindAddress("CC_VEHICLEENTRY", codeSection, codeSize, sig52, sizeof(sig52), 0, 0x4D);

		BYTE sig53[] = {0x83, 0xCD, 0xFF, 0x80, 0xF9, 0x01};
		CC_WEAPONRELOAD = FindAddress("CC_WEAPONRELOAD", codeSection, codeSize, sig53, sizeof(sig53), 0, -4);

		BYTE sig54[] = {0x81, 0xC7, 0xC4, 0x01, 0x00, 0x00, 0x81, 0xC1, 0x5F, 0xF3, 0x6E, 0x3C};
		CC_DAMAGELOOKUP = FindAddress("CC_DAMAGELOOKUP", codeSection, codeSize, sig54, sizeof(sig54), 0, -177);

		BYTE damappsig[] = {0x8B, 0x44, 0x24, 0x38, 0x66, 0x83, 0x38, 0x02};
		CC_DAMAGEAPPLICATION = FindAddress("CC_DAMAGEAPPLICATION", codeSection, codeSize, damappsig, sizeof(damappsig), 0, 0x29);

		BYTE sig55[] = {0x8D, 0x04, 0xC0, 0x8D, 0x04, 0x81, 0x8B, 0x40, 0x0C};
		CC_WEAPONASSIGN = FindAddress("CC_WEAPONASSIGN", codeSection, codeSize, sig55, sizeof(sig55), 0, 6);

		BYTE sig56[] = {0x8B, 0x48, 0x04, 0x85, 0xC9, 0x75, 0x3A};
		CC_OBJECTCREATION = FindAddress("CC_OBJECTCREATION", codeSection, codeSize, sig56, sizeof(sig56), 0, 0);

		BYTE sig_createattempt[] = {0x8B, 0xAC, 0x24, 0x28, 0x02, 0x00, 0x00, 0x8B, 0x45, 0x00};
		CC_OBJECTCREATIONATTEMPT = FindAddress("CC_OBJECTCREATIONATTEMPT", codeSection, codeSize, sig_createattempt, sizeof(sig_createattempt), 0, -14);

		BYTE sig57[] = {0x8B, 0xF0, 0x8B, 0xCF, 0x2B, 0xF7};
		CC_MAPCYCLEADD = FindAddress("CC_MAPCYCLEADD", codeSection, codeSize, sig57, sizeof(sig57), 0, 0);
		PATCH_ALLOCATEMAPNAME = FindAddress("PATCH_ALLOCATEMAPNAME", codeSection, codeSize, sig57, sizeof(sig57), 0, -10);

		BYTE sig58[] = {0x8A, 0x12, 0x88, 0x90, 0xA6, 0x02, 0x00, 0x00};
		CC_CLIENTUPDATE = FindAddress("CC_POSITIONUPDATE", codeSection, codeSize, sig58, sizeof(sig58), 0, 2);

		BYTE sig59[] = {0x30, 0x55, 0x8B, 0xEC};
		CC_EXCEPTION_HANDLER = FindAddress("CC_EXCEPTION_HANDLER", codeSection, codeSize, sig59, sizeof(sig59), 0, 1);
			
		#ifdef PHASOR_PC
			BYTE sig60[] = {0x81, 0xEC, 0x00, 0x08, 0x00, 0x00, 0x53, 0x55};
			PATCH_MAPTABLEALLOCATION = FindAddress("PATCH_MAPTABLEALLOCATION", codeSection, codeSize, sig60, sizeof(sig60), 0, 0x1B);

			BYTE sig61[] = {0x8D, 0x04, 0x40, 0x8A, 0x54, 0x81, 0x08, 0x84, 0xD2};
			PATCH_MAPTABLE = FindPtrAddress("PATCH_MAPTABLE", codeSection, codeSize, sig61, sizeof(sig61), 0, -4);

			BYTE sig62[] = {0x83, 0xE0, 0xBF};
			PATCH_MAPLOADING = FindAddress("PATCH_MAPLOADING", codeSection, codeSize, sig62, sizeof(sig62), 0, 0x3F);	
		#endif
			
		BYTE sig63[] = {0x53, 0x56, 0x57, 0xB3, 0x01};
		PATCH_NOMAPPROCESS = FindAddress("PATCH_NOMAPPROCESS", codeSection, codeSize, sig63, sizeof(sig63), 0, -12);

		BYTE sig64[] = {0x88, 0x43, 0x1E, 0x5E, 0x66, 0x83, 0x3B, 0x00}; //New update from Rad!
		PATCH_TEAMSELECTION = FindAddress("PATCH_TEAMSELECTION", codeSection, codeSize, sig64, sizeof(sig64), 0, -11);

		BYTE sig65[] = { 0x56, 0x83, 0xCE, 0xFF, 0x85, 0xC9, 0x57};
		FUNC_CREATEOBJECT = FindAddress("FUNC_CREATEOBJECT", codeSection, codeSize, sig65, sizeof(sig65), 0, -24);

		BYTE sig66[] = { 0x53, 0x8B, 0x5C, 0x24, 0x0C, 0x56, 0x8B, 0xF0, 0x33, 0xC0};
		FUNC_CREATEOBJECTQUERY = FindAddress("FUNC_CREATEOBJECTQUERY", codeSection, codeSize, sig66, sizeof(sig66), 0, -6);

		BYTE sig67[] = { 0x8B, 0x54, 0x08, 0x14, 0x57, 0x33, 0xFF };
		CC_VEHICLERESPAWN = FindAddress("CC_VEHICLERESPAWN", codeSection, codeSize, sig67, sizeof(sig67), 0, 21);

		BYTE sig68[] = {0x8B, 0xD7, 0x83, 0xEA, 0x02, 0xB1, 0x01};
		ADDR_SERVERINFO = FindPtrAddress("ADDR_SERVERINFO", codeSection, codeSize, sig68, sizeof(sig68), 0, -14);

		BYTE sig69[] = { 0x39, 0xB5, 0x04, 0x02, 0x00, 0x00};
		CC_EQUIPMENTDESTROY = FindAddress("CC_EQUIPMENTDESTROY", codeSection, codeSize, sig69, sizeof(sig69), 0, 0);

		BYTE sig70[] = {0x8B, 0xF8, 0x25, 0xFF, 0xFF, 0x00, 0x00, 0x8D, 0x04, 0x40, 0x8B, 0x44, 0x82, 0x08, 0x8B, 0x40, 0x04};
		FUNC_DESTROYOBJECT = FindAddress("FUNC_DESTROYOBJECT", codeSection, codeSize, sig70, sizeof(sig70), 0, -10);
		//CC_OBJECTDESTROY = FUNC_DESTROYOBJECT + 6;

		BYTE sig71[] = {0x83, 0xEC, 0x08, 0x53, 0x55, 0x56, 0x57, 0x6A, 0x03};
		FUNC_PLAYERASSIGNWEAPON = FindAddress("FUNC_PLAYERASSIGNWEAPON", codeSection, codeSize, sig71, sizeof(sig71), 0, 0);

		BYTE sig72[] = {0x83, 0xEC, 0x14, 0x53, 0x55, 0x8B, 0x6C, 0x24, 0x2C};
		FUNC_NOTIFY_WEAPONPICKUP = FindAddress("FUNC_NOTIFY_WEAPONPICKUP", codeSection, codeSize, sig72, sizeof(sig72), 0, 0);

		BYTE sig73[] = {0xC3, 0xCC, 0x55, 0x8B, 0xEC, 0x81, 0xEC, 0x88, 0x00, 0x00, 0x00};
		FUNC_ENTERVEHICLE = FindAddress("FUNC_ENTERVEHICLE", codeSection, codeSize, sig73, sizeof(sig73), 0, 2);

		BYTE sig74[] = {0x81, 0xEC, 0xDC, 0x00, 0x00, 0x00, 0x53, 0x56, 0x8B, 0xF0};
		FUNC_EJECTVEHICLE = FindAddress("FUNC_EJECTVEHICLE", codeSection, codeSize, sig74, sizeof(sig74), 0, -6);

		BYTE sig75[] = {0x8B, 0xCA, 0x8B, 0xD0, 0x25, 0xFF, 0xFF, 0x00, 0x00, 0x8D, 0x04, 0x40, 0x8B, 0x74, 0x81, 0x08};
		CC_VEHICLEFORCEEJECT = FindAddress("CC_VEHICLEFORCEEJECT", codeSection, codeSize, sig75, sizeof(sig75), 0, -0xE);

		BYTE sig76[] = {0xF6, 0x83, 0x08, 0x02, 0x00, 0x00, 0x40, 0x0F, 0x84};
		CC_VEHICLEUSEREJECT = FindAddress("CC_VEHICLEUSEREJECT", codeSection, codeSize, sig76, sizeof(sig76), 0, 0);

		BYTE sig77[] = {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x81, 0xEC, 0x10, 0x01, 0x00, 0x00};
		CC_HALOPRINT = FindAddress("CC_HALOPRINT", codeSection, codeSize, sig77, sizeof(sig77), 0, 0);

		BYTE sig78[] = {0x51, 0x53, 0x56, 0x57, 0x32, 0xDB};
		CC_HALOBANCHECK = FindAddress("CC_HALOBANCHECK", codeSection, codeSize, sig78, sizeof(sig78), 0, 0);

		BYTE sig79[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x08, 0x53, 0x56, 0x57, 0x55};
		FUNC_HALOEXCEPTIONHANDLER = FindAddress("FUNC_HALOEXCEPTIONHANDLER", codeSection, codeSize, sig79, sizeof(sig79), 0, 0);

		BYTE sig80[] = {0xB9, 0x53, 0x00, 0x00, 0x00, 0xF3, 0xAB};
		ADDR_BROADCASTVERSION = FindPtrAddress("ADDR_BROADCASTVERSION", codeSection, codeSize, sig80, sizeof(sig80), 0, 0x36);

		BYTE sig81[] = {0x83, 0xC8, 0xFF, 0xC3, 0x8B, 0xC1, 0xC1, 0xE0, 0x04};
		ADDR_HASHLIST = FindPtrAddress("ADDR_HASHLIST", codeSection, codeSize, sig81, sizeof(sig81), 0, 0x0a);
		ADDR_HASHLIST += 4;

		BYTE sig82[] = {0x8A, 0x4E, 0x06, 0xC0, 0xE9, 0x02, 0xF6, 0xC1, 0x01};
		ADDR_SERVERSTRUCT = FindPtrAddress("ADDR_SERVERSTRUCT", codeSection, codeSize, sig82, sizeof(sig82), 0, -26);
		
		BYTE sig83[] = {0x83, 0x7A, 0x10, 0xFF};
		CC_OBJECTCREATIONATTEMPT = FindAddress("CC_OBJECTCREATIONATTEMPT", codeSection, codeSize, sig83, sizeof(sig83), 0, 0x21, 0);
		
#ifdef PHASOR_PC
		BYTE sig85[] = {0xC6, 0x44, 0x24, 0x14, 0x00, 0xC6, 0x44, 0x24, 0x55, 0x00};
#elif PHASOR_CE
		BYTE sig85[] = {0x88, 0x5C, 0x24, 0x1C, 0x88, 0x5C, 0x24, 0x5D};
#endif
		CC_SERVERCMDATTEMPT = FindAddress("CC_SERVERCMDATTEMPT", codeSection, codeSize, sig85, sizeof(sig85), 0, -5, 0);
		ReadBytes(CC_SERVERCMDATTEMPT + 1, &ADDR_RCONPASSWORD, sizeof(ADDR_RCONPASSWORD));

		BYTE sig86[] = {0x3B, 0xC8, 0x0F, 0x94, 0xC0, 0x33, 0xDB};
		PATCH_SERVERNAME1 = FindAddress("PATCH_SERVERNAME1", codeSection, codeSize, sig86, sizeof(sig86), 0, 0x12, 0);
		PATCH_SERVERNAME2 = PATCH_SERVERNAME1 + 0x12;

		BYTE sig87[] = {0x8B, 0x49, 0x08, 0x89, 0x74, 0x24, 0x08, 0x5E};
		PATCH_CURRENTVERSION = FindAddress("PATCH_CURRENTVERSION", codeSection, codeSize, sig87, sizeof(sig87), 0, 0x10, 0);
		PATCH_ANYVERSIONCHECK1 = PATCH_CURRENTVERSION + 4;
		PATCH_ANYVERSIONCHECK2 = PATCH_ANYVERSIONCHECK1 + 0x0B;

		BYTE sig88[] = {0xC6, 0x44, 0x24, 0x29, 0x67};
		CC_HASHVALIDATE = FindAddress("CC_HASHVALIDATE", codeSection, codeSize, sig88, sizeof(sig88), 0, 0x8c, 0);
		CC_HASHVALIDATE_VALID = CC_HASHVALIDATE - 0x2f;

#ifdef PHASOR_CE
		BYTE sig89[] = {0xC6, 0x84, 0x24, 0x0B, 0x01, 0x00, 0x00, 0x00, 0x8D, 0x74, 0x24, 0x08};
		FUNC_VERIFYMAP_CE = FindAddress("FUNC_VERIFYMAP_CE", codeSection, codeSize, sig89, sizeof(sig89), 0, -33, 0);
#endif

		BYTE sig90[] = {0x83, 0x60, 0x10, 0xDF};
		DWORD func_vehiclerespawn = FindAddress("FUNC_VEHICLERESPAWN1/2", codeSection, codeSize, sig90, sizeof(sig90), 0, 0, 0);
		FUNC_VEHICLERESPAWN1 = func_vehiclerespawn + 0x11;
		FUNC_VEHICLERESPAWN2 = func_vehiclerespawn - 0x4f;

		BYTE sig91[] = {0x8B, 0x45, 0x00, 0x8B, 0x88, 0x9C, 0x0A, 0x00, 0x00};
		CC_MACHINECONNECT = FindAddress("CC_MACHINECONNECT", codeSection, codeSize, sig91, sizeof(sig91), 0, 3, 0);

#ifdef PHASOR_PC
		BYTE sig92[] = {0x8D, 0x44, 0x6D, 0x00, 0xC1, 0xE0, 0x05, 0x8D, 0x34, 0x10};
#elif PHASOR_CE
		BYTE sig92[] = {0x69, 0xC0, 0xEC, 0x00, 0x00, 0x00, 0x8D, 0x34, 0x18};
#endif
		CC_MACHINEDISCONNECT = FindAddress("CC_MACHINEDISCONNECT", codeSection, codeSize, sig92, sizeof(sig92), 0, 0, 0);

#ifdef PHASOR_PC
		BYTE sig93[] = {0x8D, 0x44, 0x24, 0x4E, 0x50};
#elif PHASOR_CE
		BYTE sig93[] = {0x8D, 0x84, 0x24, 0xC6, 0x00, 0x00, 0x00, 0x50};
#endif
		CC_MACHINEINFOFIX = FindAddress("CC_MACHINEINFOFIX", codeSection, codeSize, sig93, sizeof(sig93), 0, 0, 0);

		BYTE sig94[] = {0x81, 0xEC, 0x38, 0x04, 0x00, 0x00, 0x8B, 0x8C, 0x24, 0x44, 0x04, 0x00, 0x00, 0x53};
		FUNC_INTERSECT = FindAddress("FUNC_INTERSECT", codeSection, codeSize, sig94, sizeof(sig94), 0, 0,0);

        BYTE sig95[] = {0x8B, 0xCA, 0x81, 0xE1, 0xFF, 0xFF, 0x00, 0x00, 0x56};
        CC_OBJECTDESTROY = FindAddress("CC_DESTROY", codeSection, codeSize, sig95, sizeof(sig95), 0, 0);

        BYTE pingSig[] = {0x6A, 0x00, 0x6A, 0x35, 0x6A, 0x00};
        CC_PINGREQUEST = FindAddress("CC_PINGREQUEST", codeSection, codeSize, pingSig, sizeof(pingSig), 0, 0x0F);

		// patch the installation of other exception handlers
		BYTE instSig[] = {0x68, 0x6C, 0x03, 0x5B, 0x00, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x50, 0x64, 0x89, 0x25, 0x00, 0x00, 0x00, 0x00};
		BYTE instPatch[] = {0x68, 0x6C, 0x03, 0x5B, 0x00, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x50, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
		std::vector<DWORD> locations = FindSignatures(instSig, 0, sizeof(instSig), codeSection, codeSize);
			
		// apply all the exception patches
		for (size_t x = 0; x < locations.size(); x++)
			WriteBytes((DWORD)codeSection+locations[x], instPatch, sizeof(instPatch));
	}
}
