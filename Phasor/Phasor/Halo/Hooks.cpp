#include "Hooks.h"
#include "Addresses.h"
#include "../../Common/Common.h"
#include "Server/Server.h"
#include "Game/Game.h"
#include "Server/MapLoader.h"
#include "Game/Objects.h"
#include "Game/Damage.h"
#include "Server/Lead.h"

using namespace halo;

// Server function codecaves
// ------------------------------------------------------------------------


// Codecave for timers, safer than creating threads (hooking console chceking routine)
DWORD consoleProc_ret = 0;
__declspec(naked) void OnConsoleProcessing_CC()
{	
	__asm
	{
		pop consoleProc_ret

		pushad
		call server::OnConsoleProcessing
		popad

		PUSH EBX
		XOR EBX,EBX
		push consoleProc_ret
		CMP AL,BL
		ret
	}
}

// Codecave for hooking console events (closing etc)
DWORD conHandler_ret = 0;
__declspec(naked) void ConsoleHandler_CC()
{	
	__asm
	{
		pop conHandler_ret

		pushad

		push esi
		call server::ConsoleHandler

		popad

		mov eax, 1
		push conHandler_ret
		ret
	}
}


__declspec(naked) void OnServerCommandAttempt_CC()
{
	__asm 
	{
		add esp, 4 // we never give control back to Halo
		
#ifdef PHASOR_PC
		lea eax, dword ptr ds:[esp + 0x0C]
#elif PHASOR_CE
		lea eax, dword ptr ds:[esp + 0x14]
#endif
		pushad

		push ebp // player
		push eax
		call server::ProcessCommandAttempt

		popad

#ifdef PHASOR_PC
		POP EDI
		POP EBP
		ADD ESP,0x50
#elif PHASOR_CE
		pop edi // this will skip halo's logging, but meh
		pop ebp
		pop ebx
		add esp, 0x54
#endif
		RETN
	}
}

// Codecave for intercepting server commands
DWORD cmd_ret = 0;
static const BYTE COMMAND_PROCESSED = e_command_result::kProcessed;
__declspec(naked) void OnServerCommand_CC()
{
	__asm
	{
		// Save return address
		pop cmd_ret

		pushad

		push edi
		call server::ProcessCommand

		cmp al, COMMAND_PROCESSED
		je PROCESSED

		popad

		MOV AL,BYTE PTR DS:[EDI]
		SUB ESP,0x500
		push cmd_ret
		ret

PROCESSED:

		popad
		mov eax, 1
		ret
	}
}

// Codecave for loading maps into the cyc;e
DWORD mapload_ret = 0;
__declspec(naked) void OnMapLoading_CC()
{	
	__asm
	{
		pop mapload_ret

		pushad

		lea eax, dword ptr ds:[eax + esi]
		push eax
		call server::OnMapLoad

		cmp al, 0
		je RELOAD_MAP_DATA

		popad

		MOV ECX,DWORD PTR DS:[ESI+EAX]
		PUSH 0x3F
		push mapload_ret
		ret

RELOAD_MAP_DATA:

		popad

		// reset mapycle index
		mov eax, DWORD PTR ds:[ADDR_MAPCYCLEINDEX]
		MOV [eax], 0
		mov esi, 0 // value of ADDR_MAPCYCLEINDEX, which we just set to 0

		// get data for current map
		mov eax, dword ptr ds:[ADDR_MAPCYCLELIST]
		mov eax, [eax]
		
		//MOV ESI,DWORD PTR DS:[ADDR_MAPCYCLEINDEX]
		//mov ESI, [ESI]

		MOV ECX,DWORD PTR DS:[EAX+ESI]
		push 0x3F
		push mapload_ret
		ret
	}
}

// Game function codecaves
// ------------------------------------------------------------------------
// Codecave for detecting game ending
__declspec(naked) void OnGameEnd_CC()
{	
	__asm
	{
		pop edx // ret addr

		pushad

		push eax
		call server::OnGameEnd

		popad

		SUB ESP,0x0C                      
		PUSH 0
		push edx
		ret
	}
}

DWORD newGame_ret = 0;

// Codecave used for detecting a new game
__declspec(naked) void OnNewGame_CC()
{
	__asm
	{
		pop newGame_ret

		MOV EAX,DWORD PTR DS:[ADDR_NEWGAMEMAP]
		mov eax, [eax]

		cmp EAX, 0
		je NOT_NEW_MAP
		pushad

		lea eax, dword ptr ds:[esp + 0x24]
		mov eax, [eax]
		push eax
		call server::OnNewGame

		popad

NOT_NEW_MAP:

		push newGame_ret
		ret
	}
}

DWORD playerjoin_ret = 0;

// Codecave for people joining
__declspec(naked) void OnPlayerWelcome_CC()
{	
	__asm
	{
		pop playerjoin_ret
		#ifdef PHASOR_PC
			PUSH EAX
			MOV ESI,EBP
		#elif PHASOR_CE
			PUSH EBP
			MOV ESI,EAX
		#endif

		call dword ptr ds:[FUNC_PLAYERJOINING]
		pushad

		mov eax, [esp - 0x60]
		cmp eax, 0 // 0 when player isn't actually joining (happens if join when game just ends)
		je IGNORE_JOIN
		and eax, 0xff
		push eax
		call game::OnPlayerWelcome
IGNORE_JOIN:

		popad
		push playerjoin_ret
		ret
	}
}

DWORD leaving_ret = 0;

// Codecave used for detecting people leaving
__declspec(naked) void OnPlayerQuit_CC()
{	
	__asm
	{
		pop leaving_ret

		pushad

		and eax, 0xff
		push eax
		call game::OnPlayerQuit

		popad

		PUSH EBX
		PUSH ESI
		MOV ESI,EAX
		MOV EAX,DWORD PTR DS:[ADDR_PLAYERBASE]
		mov eax, [eax]
		push leaving_ret
		ret
	}
}

DWORD teamsel_ret = 0, selection = 0;

// Codecave for team selection
__declspec(naked) void OnTeamSelection_CC()
{
    __asm
    {
        pop teamsel_ret

        CMP BYTE PTR DS: [EBX+0x1E], 0xFF
        jne skip_halo_team_assign
        // Player doesn't have previous team, find one.
        // ecx can be modified safely (see FUNC_TEAMSELECT)
        mov ecx, FUNC_TEAMSELECT
        call ecx
        jmp phasor_team_assign
skip_halo_team_assign:
        // use previous team
        mov al, byte ptr ds: [EBX+0x1E]

phasor_team_assign:

		pushad

		mov edi, dword ptr ds:[esp + 0x38]
		push edi // ptr ptr ptr to machine struct
		movsx eax, al
		push eax
		call game::OnTeamSelection
		mov selection, eax

		popad
		mov eax, selection
		push teamsel_ret
		ret
	}
}

// Codecave for handling team changes
DWORD teamchange_ret = 0;
__declspec(naked) void OnTeamChange_CC()
{	
	__asm
	{
		pop teamchange_ret // ret addr cant safely modify

		pushad

		push ebx // team

#ifdef PHASOR_PC
		and eax, 0xff
		push eax // player
#elif	PHASOR_CE
		and edx, 0xff
		push edx // player
#endif
		
		call game::OnTeamChange

		cmp al, 0
		je DO_NOT_CHANGE

		// Allow them to change
		popad

		// overwritten code
		#ifdef PHASOR_PC
		MOVZX ESI,AL
		LEA ECX,DWORD PTR DS:[EDI+8]
		#elif PHASOR_CE
		ADD EAX, 8
		PUSH EBX
		PUSH EAX
		#endif

		push teamchange_ret // return address
		ret

DO_NOT_CHANGE:

		// Don't let them change
		popad

#ifdef PHASOR_PC
		POP EDI
#endif
		POP ESI
		POP EBX
		ADD ESP,0x1C
		ret
	}
}

// Codecave for player spawns (just before server notification)
__declspec(naked) void OnPlayerSpawn_CC()
{	
	__asm
	{
		pop ecx // can safely use ecx (see function call after codecave)

		pushad

		and ebx, 0xff
		push esi // memory object id
		push ebx // player memory id
		call game::OnPlayerSpawn

		popad

		// orig code
		push 0x7ff8
		push ecx
		ret
	}
}

DWORD playerSpawnEnd_ret = 0;

// Codecave for player spawns (after server has been notified).
__declspec(naked) void OnPlayerSpawnEnd_CC()
{	
	__asm
	{
		pop playerSpawnEnd_ret

		mov ecx, FUNC_AFTERSPAWNROUTINE
		call ecx
		add esp, 0x0C

		pushad

		and ebx, 0xff

		push esi // memory object id
		push ebx // player memory id
		call game::OnPlayerSpawnEnd

		popad
		push playerSpawnEnd_ret
		ret
	}
}

DWORD objcreation_ret = 0;

// Codecave for modifying objects as they're created
__declspec(naked) void OnObjectCreation_CC()
{
	__asm
	{
		pop objcreation_ret

		pushad

		push ebx
		call game::OnObjectCreation

		popad

		MOV ECX,DWORD PTR DS:[EAX+4]
		TEST ECX,ECX
		push objcreation_ret
		ret
	}
}

DWORD objcreationattempt_ret = 0;

// Codecave for blocking objects from being created
__declspec(naked) void OnObjectCreationAttempt_CC()
{
	__asm
	{
		pop objcreationattempt_ret

		pushad

		mov eax, [ESP + 0x24]
		push eax // creation description
        mov eax, [ESP + 0x38]
        push eax // player struct (if any)
		call game::OnObjectCreationAttempt

		cmp al, 1
		je ALLOW_CREATION

		popad
		mov eax, -1
		ret

ALLOW_CREATION:
		popad

		SUB ESP,0x21C
		push objcreationattempt_ret
		ret
	}
}

DWORD objdestroy_ret = 0;
__declspec(naked) void OnObjectDestroy_CC()
{
	__asm
	{
		pop objdestroy_ret

		pushad

		push edx
		call game::OnObjectDestroy

		popad

        MOV ECX, EDX
        AND ECX, 0x0000FFFF

		push objdestroy_ret
		ret
	}
}


DWORD wepassignment_ret = 0, wepassign_val = 0;

// Codecave for handling weapon assignment to spawning players
__declspec(naked) void OnWeaponAssignment_CC()
{
	__asm
	{
		pop wepassignment_ret

		pushad

		mov edi, [esp + 0xFC]
		and edi, 0xff

		mov ecx, [esp + 0x2c]
		push ecx // order
		push eax // curweapon
		push esi // owner
		push edi // player
		call game::OnWeaponAssignment
		mov wepassign_val, eax

		popad

		mov eax, wepassign_val // id of weapon to make
		//MOV EAX,DWORD PTR DS:[EAX+0x0C] // original instruction
		CMP EAX,-1
		push wepassignment_ret		
		ret
	}
}

DWORD objInteraction_ret = 0;

// Codecave for handling object pickup interactions
__declspec(naked) void OnObjectInteraction_CC()
{
	__asm
	{
		pop objInteraction_ret

		pushad

		mov eax, dword ptr ds:[esp + 0x38]
		mov edi, dword ptr ds:[esp + 0x3c]

		and eax, 0xff
		push edi
		push eax
		call game::OnObjectInteraction

		cmp al, 0
		je DO_NOT_CONTINUE

		popad

		PUSH EBX
		MOV EBX,DWORD PTR SS:[ESP+0x20]
		push objInteraction_ret
		ret

DO_NOT_CONTINUE:

		popad
		add esp, 0x14
		ret
	}
}

// Codecave for server chat
__declspec(naked) void OnChat_CC()
{
	__asm
	{
		// don't need the return address as we skip this function
		add esp, 4

		pushad

		#ifdef PHASOR_PC
		mov eax, dword ptr ds:[esp + 0x2c]
		and eax, 0xff // other 3 bytes aren't used for player number and can be non-zero
		mov [esp + 0x2c], eax
		lea eax, dword ptr ds:[esp + 0x28]
		#elif PHASOR_CE
		mov eax, dword ptr ds:[esp + 0x40]
		and eax, 0xff // other 3 bytes aren't used for player number and can be non-zero
		mov [esp + 0x40], eax
		lea eax, dword ptr ds:[esp + 0x3C]
		#endif
		
		push eax
        push esi // machine entry
		call game::OnChat

		popad

		POP EDI
		#ifdef PHASOR_PC
		ADD ESP,0x224
		#elif PHASOR_CE
		ADD ESP, 0x228
		#endif
		ret
	}
}

// Codecave for player position updates
__declspec(naked) void OnClientUpdate_CC()
{
	__asm
	{
		pop edi // can safely use edi as its poped in return stub

		// Execute the original code
		MOV BYTE PTR DS:[EAX+0x2A6],DL
		JE L008
		MOV DWORD PTR DS:[EAX+0x4BC],EBP
		MOV BYTE PTR DS:[EAX+0x4B8],0x1
		jmp START_PROCESSING
L008:
		MOV BYTE PTR DS:[EAX+0x4B8],0x0
START_PROCESSING:

		pushad
		mov eax, [esp + 0x20]
		push eax // player's object
		call server::OnClientUpdate

		popad

		// return out of function
		POP EDI
		POP ESI
		POP EBP
		ret
	}
}

DWORD dmglookup_ret = 0;

// Codecaves for handling weapon damage
__declspec(naked) void OnDamageLookup_CC()
{
	__asm
	{
		pop dmglookup_ret

		pushad

	//	mov esi, [ebp + 0x0C] // object causing the damage
	//	lea ecx, dword ptr ds:[edx + eax] // table entry for the damage tag
	//	mov edi, [esp + 0xcc] // object taking the damage
	//	
		mov esi, [esp + 0x24]
		lea edi, [esp + 0x28]

		push edi
		push esi
		call OnDamageLookup

		cmp al, 1
		je ALLOW_DAMAGE

		popad
		ret
ALLOW_DAMAGE:
		popad

		SUB ESP,0x94
		push dmglookup_ret
		ret
	}
}

DWORD dmgapplication_ret = 0;
__declspec(naked) void OnDamageApplication_CC()
{
	__asm
	{
		pop dmgapplication_ret

		pushad

		mov edi, [esp + 0x58]
		mov dl, byte ptr ds:[edi]

		// only a backtap if it's melee damage.
		cmp dl, 2
		je POSSIBLE_BACKTAP
		mov al, 0 // can't be a backtap, not melee damage

POSSIBLE_BACKTAP:
		movzx eax, al
		mov ecx, [esp + 0x5c] // receiver

		push eax // backtap?
		push esi // hit location
		push ecx // receiver
		push ebp // dmg info
		call OnDamageApplication

		cmp al, 1
		je ALLOW_DMG_APP

		popad

		POP EDI
		POP ESI
		POP EBP
		POP EBX
		ADD ESP,0x94
		RETN


ALLOW_DMG_APP:
		popad
		// orig code
		CMP BYTE PTR SS:[ESP+0x40],1

		push dmgapplication_ret
		ret
	}
}

DWORD OnVehicleEntry_ret = 0;
__declspec(naked) void OnVehicleEntry_CC()
{
	__asm
	{
		pop OnVehicleEntry_ret

		pushad
		pushfd

		mov eax, [ebp + 0x8]
		and eax, 0xff

		push eax
		call game::OnVehicleEntry

		cmp al, 1

		je ALLOW_VEHICLE_ENTRY

		popfd
		popad

		MOV AL,BYTE PTR SS:[EBP-1]

		POP EDI
		POP ESI
		POP EBX
		MOV ESP,EBP
		POP EBP	

		ret // don't let them change

ALLOW_VEHICLE_ENTRY:

		popfd
		popad

		MOV DWORD PTR SS:[EBP-8],-1
		push OnVehicleEntry_ret
		ret
	}
}

DWORD ondeath_ret = 0;
bool show_kill_msg = true;
// Codecave for handling player deaths
__declspec(naked) void OnDeath_CC()
{
	__asm
	{
		pop ondeath_ret

		pushad

		lea eax, dword ptr ds:[esp + 0x38] // victim
		mov eax, [eax]
		and eax, 0xff
		lea ecx, dword ptr ds:[esp + 0x34] // killer
		mov ecx, [ecx]
		and ecx, 0xff

		push esi // mode of death
		push eax // victim
		push ecx // killer
		call game::OnPlayerDeath
		mov show_kill_msg, al
		popad

		mov bl, show_kill_msg

		PUSH EBP
		MOV EBP,DWORD PTR SS:[ESP+0x20]
		push ondeath_ret
		ret
	}
}

DWORD killmultiplier_ret = 0;

// Codecave for detecting double kills, sprees etc
__declspec(naked) void OnKillMultiplier_CC()
{
	__asm
	{
		pop killmultiplier_ret

		pushad

		push esi // muliplier
		and edi, 0xff
		push edi // player
		call game::OnKillMultiplier

		popad

		MOV EAX,DWORD PTR SS:[ESP+0x1C]
		PUSH EBX
		push killmultiplier_ret
		ret
	}
}

// Codecave for handling weapon reloading
DWORD weaponReload_ret = 0;
__declspec(naked) void OnWeaponReload_CC()
{
	__asm
	{
		pop weaponReload_ret

		pushad

		mov ecx, [esp + 0x40]
		push ecx
		call game::OnWeaponReload

		cmp al, 1
		je ALLOW_RELOAD

		popad

		AND DWORD PTR DS:[EAX+0x22C],0xFFFFFFF7
		POP EDI
		POP ESI
		POP EBP
		POP EBX
		ADD ESP,0xC
		ret

ALLOW_RELOAD:

		popad

		MOV CL,BYTE PTR SS:[ESP+0x28]
		OR EBP,0xFFFFFFFF
		push weaponReload_ret
		ret
	}
}

// used to control object respawning
DWORD vehires_ret = 0;
__declspec(naked) void OnVehicleRespawn_CC()
{
	__asm
	{
		pop vehires_ret

		pushad
		push ebx // object's memory address
		push esi // object's id
		call objects::VehicleRespawnCheck // returns true if we should respawn, false if not
		cmp al, 2
		je OBJECT_DESTROYED
		cmp al, 1 // return to a JL statement, jump if not respawn
		popad
		push vehires_ret
		ret

OBJECT_DESTROYED:
		popad

		mov eax, 1
		pop edi
		pop esi
		pop ebx
		mov esp,ebp
		pop ebp
		ret
	}
}

// used to control itmc destruction
DWORD itmcdes_ret = 0;
__declspec(naked) void OnEquipmentDestroy_CC()
{
	__asm
	{
		pop itmcdes_ret

		pushad
		push ebp // equipment's memory address
		push ebx // equipment's memory id
		push esi // tick count of when the item is due for destruction
		call objects::EquipmentDestroyCheck // returns true if should destroy
		xor al, 1 // 1 -> 0, 0 -> 1
		cmp al, 1 // returns to a JGE so if al < 1 item is destroyed
		popad

		push itmcdes_ret
		ret
	}
}

DWORD vehfeject_ret = 0;
__declspec(naked) void OnVehicleForceEject_CC()
{
	__asm
	{
		pop vehfeject_ret

		pushad
		push 1 // force eject
		push ebx 
		call game::OnVehicleEject // false - don't eject

		cmp al, 1
		je DO_FORCE_EJECT

		// don't let them eject
		popad
		sub vehfeject_ret, 0x2d //56e6d2-56e6a5 = 0x2d
		cmp al, al // to force jump

		push vehfeject_ret
		ret

DO_FORCE_EJECT:
		popad

		CMP WORD PTR DS:[EBX+0x2F0],0x0FFFF
		push vehfeject_ret
		ret
	}
}

DWORD vehueject_ret = 0;
__declspec(naked) void OnVehicleUserEject_CC()
{	
	__asm
	{
		pop vehueject_ret

		TEST BYTE PTR DS:[EBX+0x208],0x40
		je NOT_EJECTING

		pushad
		push 0
		push ebx // not a forceable ejection
		call game::OnVehicleEject // false - don't eject

		cmp al, 1
		je DO_USER_EJECT

		popad 
		cmp al, al // force the jump (stop ejection)

		push vehueject_ret
		ret

DO_USER_EJECT:
		popad
		TEST BYTE PTR DS:[EBX+0x208],0x40
NOT_EJECTING:
		
		push vehueject_ret
		ret
	}
}

__declspec(naked) void OnHaloPrint_CC()
{
	__asm
	{
		add esp,4 // we don't execute the func so dont need ret addr

		pushad
		push eax
		call server::OnHaloPrint
		popad
		
		ret
	}
}

DWORD halobancheck_ret = 0;
__declspec(naked) void OnHaloBanCheck_CC()
{
	__asm
	{
		pop halobancheck_ret

		pushad
		mov eax, [esp + 0x28]
		push eax
		push edi
		call server::OnHaloBanCheck
		cmp al, 1 // not banned
		JE PLAYER_NOT_BANNED
		popad
		mov al, 1 // banned
		ret
PLAYER_NOT_BANNED:
		popad

		PUSH ECX
		PUSH EBX
		PUSH ESI
		PUSH EDI
		XOR BL,BL
		push halobancheck_ret
		ret
	}
}

DWORD hashcheck_ret = 0;
__declspec(naked) void OnHaloHashCheck_CC() 
{
	__asm
	{
		pop hashcheck_ret

			pushad

			push eax // errmsg
			push esi // request struct
			call server::OnHashValidation

			popad

			ADD ESP,0x0C
			MOV DWORD PTR DS:[ESI+0x44],EAX


			push hashcheck_ret
			ret
	}
}

DWORD hashcheck_valid_ret = 0;
__declspec(naked) void OnHaloHashCheckValid_CC() 
{
	__asm
	{
		pop hashcheck_valid_ret

		MOV DWORD PTR DS:[ESI+0x38],1
		
		pushad

		push eax // errmsg
		push esi // request struct
		call server::OnHashValidation
		
		popad

		push hashcheck_valid_ret
		ret
	}
}

DWORD onmachineconnect_ret;
__declspec(naked) void OnMachineConnect_CC()
{
	__asm
	{
		pop onmachineconnect_ret

		pushad

		push ebx // index in machine table
		call server::OnMachineConnect

		popad

		MOV ECX,DWORD PTR DS:[EAX+0xA9C]

		push onmachineconnect_ret
		ret
	}
}

DWORD onmachinedisconnect_ret;
__declspec(naked) void OnMachineDisconnect_CC()
{
	__asm
	{
		pop onmachinedisconnect_ret

		pushad

#ifdef PHASOR_PC
		push ebp // machine index in table
#elif PHASOR_CE
		push eax // machine index in table
#endif
		call server::OnMachineDisconnect
		
		popad

#ifdef PHASOR_PC
		LEA EAX,DWORD PTR SS:[EBP+EBP*0x02]
		SHL EAX,0x05
#elif PHASOR_CE
		IMUL EAX,EAX,0x0EC
#endif
		push onmachinedisconnect_ret
		ret
	}
}

// fixes halo bugs where it doesn't check hash/challenge and player name
// is null terminated >.>
DWORD machineinfofix_ret = 0;
__declspec(naked) void OnMachineInfoFix_CC()
{
	__asm
	{
		pop machineinfofix_ret

#ifdef PHASOR_PC
		LEA EAX,DWORD PTR SS:[ESP+0x4E]
#elif PHASOR_CE
		LEA EAX,DWORD PTR SS:[ESP+0xC6]
#endif
		push eax

		pushad

		push eax
		call server::OnMachineInfoFix

		popad

		push machineinfofix_ret
		ret
	}
}

DWORD projmove_ret;
__declspec(naked) void OnProjectileMove_CC()
{
    __asm 
    {
        pop projmove_ret

        pushad

        mov eax, [ESP + 0x24]
        push eax
        call halo::server::lead::OnProjectileMove

        popad

        PUSH EBP
        MOV EBP, ESP
        AND ESP, 0xFFFFFFF8

        push projmove_ret
        ret
    }
}

__declspec(naked) void OnProjectileMoveRet_CC()
{
    __asm
    {
        add esp, 4

        pushad
        call halo::server::lead::OnProjectileMoveRet
        popad

        mov esp, ebp
        pop ebp
        ret
    }
}

DWORD raycast_ret;
__declspec(naked) void OnRayCast_CC()
{
    __asm
    {
        pop raycast_ret

        // todo: stop using pushad/popad at some point
        pushad

        // +0x20 = return address from RayCast
        
        mov eax, [esp + 0x34]
        push eax // s_intersection_output
        mov eax, [esp + 0x34]
        push eax // ignore object
        mov eax, [esp + 0x34]
        push eax // dir
        mov eax, [esp + 0x34]
        push eax // pos
        mov eax, [esp + 0x34]
        push eax // flags
        call halo::server::lead::OnRayCast

        popad

        sub esp, 0x438
        push raycast_ret
        ret
    }
}

_declspec(naked) void OnRayCastRet_CC()
{
    __asm
    {
        add esp, 4
        pushad
        call halo::server::lead::OnRayCastRet
        popad

        ADD ESP, 0x438

        ret
    }
}

__declspec(naked) void OnTickSleep_CC()
{
    __asm
    {
        mov ebx, Sleep

        pushad
        call halo::server::lead::OnTick
        popad

        ret
    }
}

namespace halo
{
	using namespace Common;
	// Installs all codecaves and applies all patches
	void InstallHooks()
	{
		// Patches
		// ----------------------------------------------------------------
		// 
		// Ensure we always decide the team
		BYTE nopSkipSelection[2] = {0x90, 0x90};
		WriteBytes(PATCH_TEAMSELECTION, &nopSkipSelection, 2);

		// Stop the server from processing map additions (sv_map, sv_mapcycle_begin)
		BYTE mapPatch[] = {0xB0, 0x01, 0xC3};
		WriteBytes(PATCH_NOMAPPROCESS, &mapPatch, sizeof(mapPatch));

#ifdef PHASOR_PC
		// Make Phasor control the loading of a map
		DWORD addr = (DWORD)UlongToPtr(server::maploader::GetLoadingMapBuffer());
		WriteBytes(PATCH_MAPLOADING, &addr, 4);

		// Set the map table
		DWORD table = server::maploader::GetMapTable();
		WriteBytes(PATCH_MAPTABLE, &table, 4);

		// Remove where the map table is allocated/reallocated by the server
		BYTE nopPatch[0x3E];
		memset(nopPatch, 0x90, sizeof(nopPatch));
		WriteBytes(PATCH_MAPTABLEALLOCATION, &nopPatch, sizeof(nopPatch));	
#endif

		BYTE nopServerName[] = {0x90, 0x90};
		WriteBytes(PATCH_SERVERNAME1, nopServerName, sizeof(nopServerName));
		WriteBytes(PATCH_SERVERNAME2, nopServerName, sizeof(nopServerName));
		
		// Server hooks
		// ----------------------------------------------------------------
		// 
		// Codecave for timers, safer than creating threads (hooking console checking routine)
		CreateCodeCave(CC_CONSOLEPROC, 5, OnConsoleProcessing_CC);

		// Codecave for hooking console events (closing etc)
		CreateCodeCave(CC_CONSOLEHANDLER, 5, ConsoleHandler_CC);

		// Codecave to intercept server commands
		CreateCodeCave(CC_SERVERCMD, 8, OnServerCommand_CC);
		CreateCodeCave(CC_SERVERCMDATTEMPT, 5, OnServerCommandAttempt_CC);

		// Codecave used to load non-default maps
		CreateCodeCave(CC_MAPLOADING, 5, OnMapLoading_CC);
		 
		// Game Hooks
		// ----------------------------------------------------------------
		//
		// Codecave for detecting game ending
		CreateCodeCave(CC_GAMEEND, 5, OnGameEnd_CC);

		// Codecave used to detect a new game
		CreateCodeCave(CC_NEWGAME, 5, OnNewGame_CC);

		// Codecave called when a player joins
		CreateCodeCave(CC_PLAYERWELCOME, 8, OnPlayerWelcome_CC);

		// Codecave used to detect people leaving
		CreateCodeCave(CC_PLAYERQUIT, 9, OnPlayerQuit_CC);

		// Codecave used to decide the player's team
		CreateCodeCave(CC_TEAMSELECTION, 5, OnTeamSelection_CC);

		// Codecave for handling team changes
		#ifdef PHASOR_PC
		CreateCodeCave(CC_TEAMCHANGE, 6, OnTeamChange_CC);
		#elif PHASOR_CE
		CreateCodeCave(CC_TEAMCHANGE, 5, OnTeamChange_CC);
		#endif

		// Codecaves for detecting player spawns
		CreateCodeCave(CC_PLAYERSPAWN, 5, OnPlayerSpawn_CC);
		CreateCodeCave(CC_PLAYERSPAWNEND, 8, OnPlayerSpawnEnd_CC);

		// Codecave called when a weapon is created
		CreateCodeCave(CC_OBJECTCREATION, 5, OnObjectCreation_CC);
		CreateCodeCave(CC_OBJECTCREATIONATTEMPT, 6, OnObjectCreationAttempt_CC);
		CreateCodeCave(CC_OBJECTDESTROY, 8, OnObjectDestroy_CC);

		// Codecave for handling weapon assignment to spawning players
		CreateCodeCave(CC_WEAPONASSIGN, 6, OnWeaponAssignment_CC);

		// Codecave for interations with pickable objects
		CreateCodeCave(CC_OBJECTINTERACTION, 5, OnObjectInteraction_CC);
		
		// Codecave for position updates
		CreateCodeCave(CC_CLIENTUPDATE, 6, OnClientUpdate_CC);

		// Codecave for handling damage being done
		CreateCodeCave(CC_DAMAGELOOKUP, 6, OnDamageLookup_CC);
		CreateCodeCave(CC_DAMAGEAPPLICATION, 5, OnDamageApplication_CC);

		// Codecave for server chat
		CreateCodeCave(CC_CHAT, 7, OnChat_CC);

		// Codecave for handling vehicle entry
		CreateCodeCave(CC_VEHICLEENTRY, 7, OnVehicleEntry_CC);

		// Codecave for handling weapon reloading
		CreateCodeCave(CC_WEAPONRELOAD, 7, OnWeaponReload_CC);

		// Codecave for detecting player deaths
		CreateCodeCave(CC_DEATH, 5, OnDeath_CC);

		// Codecave for detecting double kills, sprees etc
		CreateCodeCave(CC_KILLMULTIPLIER, 5, OnKillMultiplier_CC);

		// used to control whether or not objects respawn
		CreateCodeCave(CC_VEHICLERESPAWN, 32, OnVehicleRespawn_CC);
		CreateCodeCave(CC_EQUIPMENTDESTROY, 6, OnEquipmentDestroy_CC);
		
		// Codecaves for detecting vehicle ejections
		CreateCodeCave(CC_VEHICLEFORCEEJECT, 8, OnVehicleForceEject_CC);
		CreateCodeCave(CC_VEHICLEUSEREJECT, 7, OnVehicleUserEject_CC);
	
		// Machine connect/disconnect
		CreateCodeCave(CC_MACHINECONNECT, 6, OnMachineConnect_CC);
#ifdef PHASOR_PC
		CreateCodeCave(CC_MACHINEDISCONNECT, 7, OnMachineDisconnect_CC);
#elif PHASOR_CE
		CreateCodeCave(CC_MACHINEDISCONNECT, 6, OnMachineDisconnect_CC);
#endif
		// Generic codecaves
		CreateCodeCave(CC_HALOPRINT, 6, OnHaloPrint_CC);
		CreateCodeCave(CC_HALOBANCHECK, 6, OnHaloBanCheck_CC);
		CreateCodeCave(CC_HASHVALIDATE, 6, OnHaloHashCheck_CC);
		CreateCodeCave(CC_HASHVALIDATE_VALID, 7, OnHaloHashCheckValid_CC);
#ifdef PHASOR_PC
		CreateCodeCave(CC_MACHINEINFOFIX, 5, OnMachineInfoFix_CC);
#elif PHASOR_CE
		CreateCodeCave(CC_MACHINEINFOFIX, 8, OnMachineInfoFix_CC);
#endif
		//CreateCodeCave(CC_VERSIONBROADCAST, 6, OnVersionBroadcast_CC);

		//CreateCodeCave(0x005112d4, 5, OnJoinCheck_CC);
		//
		//BYTE maxCmp[] = {0x12};
		//WriteBytes(0x5112A9, &maxCmp, sizeof(maxCmp));
		//
		//BYTE maxCmp1[] = {0x83, 0xC4, 0x0C, 0xC3, 0x90, 0x90, 0x90, 0x90};
		//WriteBytes(0x005152FD , &maxCmp1, sizeof(maxCmp1));
		//
		//BYTE curCmp[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
		//WriteBytes(0x00512479 , &curCmp, sizeof(curCmp));

        // nolead
        CreateCodeCave(FUNC_INTERSECT, 6, OnRayCast_CC);
        CreateCodeCave(CC_INTERSECT_RET1, 7, OnRayCastRet_CC);
        CreateCodeCave(CC_INTERSECT_RET2, 7, OnRayCastRet_CC);
        CreateCodeCave(CC_INTERSECT_RET3, 7, OnRayCastRet_CC);
        CreateCodeCave(CC_PROJMOVE, 6, OnProjectileMove_CC);
        CreateCodeCave(CC_PROJMOVE_RET1, 4, OnProjectileMoveRet_CC);
        CreateCodeCave(CC_PROJMOVE_RET2, 4, OnProjectileMoveRet_CC);
        CreateCodeCave(CC_ONTICKSLEEP, 6, OnTickSleep_CC);

		// I want to remove haloded's seh chain so that I can get extra exception
		// information (passed to the unhandled exception filter)
		#pragma pack(push, 1)

		struct sehEntry
		{
			sehEntry* next;
			LPBYTE hndFunc;
		};
		#pragma pack(pop)

		// Get the first entry in the chain
		sehEntry* exceptionChain = 0;
		__asm
		{
			pushad
			MOV EAX,DWORD PTR FS:[0]
			mov exceptionChain, eax
			popad
		}

		if (exceptionChain)
		{
			// loop through the exception chain and remove any references to
			// 5B036C (should do a sig search for this value.. meh)
			sehEntry* last = 0, *end = (sehEntry*)0xffffffff;
			while (exceptionChain != end) {
				if (exceptionChain->hndFunc == (LPBYTE)FUNC_HALOEXCEPTIONHANDLER) {
					// remove this entry
					if (last) {
						last->next = exceptionChain->next;
						exceptionChain = last;
					} else if (exceptionChain->next != end) { // only overwrite if there is one to go to
						exceptionChain->hndFunc = exceptionChain->next->hndFunc;
						exceptionChain->next = exceptionChain->next->next;
					}
				}
				last = exceptionChain;			
				exceptionChain = exceptionChain->next;
			}
		}
	}
}