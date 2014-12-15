#include "../Addresses.h"
#include "PlayerHistory.h"
#include "PlayerInfo.h"
#include "LeadControl.h"
#include "structs.h"

bool operator==(const ident &ident1, const ident &ident2) {
	return ident1.ID == ident2.ID && ident1.index == ident2.index;
}

#define PLAYER_HEADER_ADDR (0x4029CE90)

#define OBJECT_HEADER_ADDR (0x4005062C)

Object_Table_Header *object_table_header = (Object_Table_Header *)OBJECT_HEADER_ADDR;
Static_Player_Header *static_player_header = (Static_Player_Header *)PLAYER_HEADER_ADDR;

PlayerHistory<64> playerHistories[16];
PlayerInfo<8> playerInfos[16];

short default_lead = -1;
short Player_Lead[16] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
bool useAveragePing = false;

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
	for (short i = 0; i < static_player_header->MaxSlots; ++i) {
		Static_Player& staticPlayer = static_player_header->Players[i];
		if (staticPlayer.PlayerID == 0)
		{
			playerHistories[i].reset();
			playerInfos[i].reset();
			continue;
		}

		if (setPings) {
			playerInfos[i].addPing(staticPlayer.Ping);
		}

		ident bipedIdent = { -1 };
		bipedIdent = staticPlayer.CurrentBiped;

		if (bipedIdent.index == -1)
		{
			playerHistories[i].reset();
			continue;
		}

		AObject* player = object_table_header->Objects[bipedIdent.index].Offset;

		playerHistories[i].addData(player);

	}
}

void __stdcall dUpdateObject(ident id)
{
	AObject* object = object_table_header->Objects[id.index].Offset;
	for (short i = 0; i < 16; ++i) {

		if (Player_Lead[i] == -2 || (Player_Lead[i] == -1 && default_lead == -1)) continue;
		
		short lead = 0;
		if (default_lead != -1) lead = default_lead;
		if (Player_Lead[i] != -1) lead = Player_Lead[i];

		Static_Player& staticPlayer = static_player_header->Players[i];
		if (staticPlayer.PlayerID == 0 || staticPlayer.CurrentBiped.index == -1) continue;
		if (staticPlayer.CurrentBiped == object->Owner
		&& object->Vehicle.index == -1) {

			vect3 playerPosses[16];

			//use current ping
			short ping = staticPlayer.Ping;

			//use average ping
			if (useAveragePing) ping = playerInfos[i].getAveragePing();

			// move back all other players (1st loop)
			for (short j = 0; j < 16; ++j) {
				Static_Player& otherStaticPlayer = static_player_header->Players[j];
				if (otherStaticPlayer.PlayerID == 0 || otherStaticPlayer.CurrentBiped.index == -1 || j == i) continue;
				AObject* otherPlayer = object_table_header->Objects[otherStaticPlayer.CurrentBiped.index].Offset;
				
				//store position
				playerPosses[j] = otherPlayer->World;

				//thank you nuggets for showing me that ping - lead would actually work.
				playerHistories[j].readDataIntoPlayer(max(0, ping - lead), otherPlayer);

				updatePhysics(otherStaticPlayer.CurrentBiped);
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

			finish_label:
				add esp, 4

				popad
			}

			// restore position for all other players (2nd loop)
			for (short j = 0; j < 16; ++j) {
				Static_Player& otherStaticPlayer = static_player_header->Players[j];
				if (otherStaticPlayer.PlayerID == 0 || otherStaticPlayer.CurrentBiped.index == -1 || j == i) continue;
				AObject* otherPlayer = object_table_header->Objects[otherStaticPlayer.CurrentBiped.index].Offset;
				otherPlayer->World = playerPosses[j];

				updatePhysics(otherStaticPlayer.CurrentBiped);
			}
		}
	}
}