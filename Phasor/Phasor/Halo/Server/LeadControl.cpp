#include "../Addresses.h"
#include "../Game/Objects.h"
#include "../Player.h"
#include "PlayerHistory.h"
#include "PlayerInfo.h"
#include "LeadControl.h"

PlayerHistory<64> playerHistories[16];
PlayerInfo<8> playerInfos[16];

short default_lead = -1;
short Player_Lead[16] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
bool useAveragePing = false;

using namespace halo;

#define PLAYER_HEADER_ADDR (0x4029CE90)

#define OBJECT_HEADER_ADDR (0x4005062C)

objects::s_halo_object_table *object_table = (objects::s_halo_object_table *)OBJECT_HEADER_ADDR;
s_player_table *player_table = (s_player_table *)PLAYER_HEADER_ADDR;

void updatePhysics(ident id)
{		
	__asm
	{
		pushad
		mov eax, id
		push eax
		call dword ptr ds : [FUNC_UPDATE_PHYSICS]
		add esp, 4
		popad
	}
}

void __stdcall dUpdateAllObjects()
{
	static size_t counter = 0;
	bool setPings = false;

	counter++;
	if (counter >= 30) {
		counter = 0;
		setPings = true;
	}

	// store player histories and pings
	for (short i = 0; i < player_table->header.max_size; ++i) {
		s_player_structure& staticPlayer = player_table->players[i];
		if (staticPlayer.playerJoinCount == 0)
		{
			playerHistories[i].reset();
			playerInfos[i].reset();
			continue;
		}

		if (setPings) {
			playerInfos[i].addPing(staticPlayer.ping);
		}

		ident bipedIdent = staticPlayer.object_id;

		if (bipedIdent == -1)
		{
			playerHistories[i].reset();
			continue;
		}

		objects::s_halo_object* player = object_table->entries[bipedIdent.slot].base;

		playerHistories[i].addData(player);

	}
}

void __stdcall dUpdateObject(ident id)
{
	objects::s_halo_object* object = object_table->entries[id.slot].base;
	objects::s_halo_unit* unit = (objects::s_halo_unit*)object;
	for (short i = 0; i < 16; ++i) {

		if (Player_Lead[i] == -2 || (Player_Lead[i] == -1 && default_lead == -1)) continue;
		short lead = 0;
		if (default_lead != -1) lead = default_lead;
		if (Player_Lead[i] != -1) lead = Player_Lead[i];

		s_player_structure& staticPlayer = player_table->players[i];
		if (staticPlayer.playerJoinCount == 0 || staticPlayer.object_id.slot == -1) continue;

		if (unit->throwing_grenade_state != 0) continue; //if player is throwing a grenade, skip him.

		if (staticPlayer.object_id == object->ownerObject && object->vehicleId.slot == -1){

			vect3d playerPosses[16];

			//use current ping
			short ping = staticPlayer.ping;

			//use average ping
			if (useAveragePing) ping = playerInfos[i].getAveragePing();

			// move back all other players (1st loop)
			for (short j = 0; j < 16; ++j) {
				s_player_structure& otherStaticPlayer = player_table->players[j];
				if (otherStaticPlayer.playerJoinCount == 0 || otherStaticPlayer.object_id.slot == -1 || j == i) continue;
				objects::s_halo_object* otherPlayer = object_table->entries[id.slot].base;

				//store position
				playerPosses[j] = otherPlayer->location;

				//thank you nuggets for showing me that ping - lead would actually work.
				playerHistories[j].readDataIntoPlayer(max(0, ping - lead), otherPlayer);

				updatePhysics(otherStaticPlayer.object_id);
			}

			//thanks oxide for remembering to go back to the function
			DWORD goBackToFunc = CC_UPDATEOBJECT + 7;

			__asm {

				pushad

				push id // fake a call
				push finish_label

				PUSH ECX
				MOV ECX, 0x4005062C
				MOV EDX, DWORD PTR DS : [ECX + 0x34]
				PUSH EBX
				MOV EBX, id
				MOV EAX, EBX
				AND EAX, 0x0000FFFF

				jmp goBackToFunc

				finish_label :
					add esp, 4

					popad
				}

			// restore position for all other players (2nd loop)
			for (short j = 0; j < 16; ++j) {
				s_player_structure& otherStaticPlayer = player_table->players[j];
				if (otherStaticPlayer.playerJoinCount == 0 || otherStaticPlayer.object_id.slot == -1 || j == i) continue;
				objects::s_halo_object* otherPlayer = object_table->entries[id.slot].base;
				otherPlayer->location = playerPosses[j];

				updatePhysics(otherStaticPlayer.object_id);
			}
		}
	}
}