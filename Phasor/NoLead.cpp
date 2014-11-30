/********************************************************************************
Copyright (C) 2012 PaulusT

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************************/

#include "structs.h"
#include <fstream>
#include "PlayerHistory.h"
#include "PlayerInfo.h"
#include "Addresses.h"

// TODO: Use mouse_accel? for ping multiplier?
// TODO: multiplayer_draw_teammates_names for enable average ping?

bool operator==(const ident &ident1, const ident &ident2) {
	return ident1.ID == ident2.ID && ident1.index == ident2.index;
}

Object_Table_Header *object_table_header = (Object_Table_Header *)ADDR_OBJECTBASE;
Static_Player_Header *static_player_header = (Static_Player_Header *)ADDR_PLAYERBASE;


typedef char(*tFuncOneObject)(ident object);

typedef char(*tFuncNoArgs)();

tFuncOneObject oUpdateObject = 0;
tFuncNoArgs oUpdateAllObjects = 0;
tFuncOneObject updatePhysics = (tFuncOneObject)FUNC_UPDATE_PHYSICS;

PlayerHistory<64> playerHistories[16];
PlayerInfo<8> playerInfos[16];

std::ofstream logfile("logP.txt");

//used to be char not void
void __cdecl dUpdateAllObjects()
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
	//char res = oUpdateAllObjects();

	//return res;
}

//used to be char not void
void __cdecl dUpdateObject(ident id)
{
	AObject* object = object_table_header->Objects[id.index].Offset;

	for (short i = 0; i < 16; ++i) {
		Static_Player& staticPlayer = static_player_header->Players[i];
		if (staticPlayer.PlayerID == 0 || staticPlayer.CurrentBiped.index == -1) continue;

		if (staticPlayer.CurrentBiped == object->Owner
			&& object->Vehicle.index == -1) {

			vect3 playerPosses[16];
			vect3 playerVels[16];

			// move back all other players
			for (short j = 0; j < 16; ++j) {
				Static_Player& otherStaticPlayer = static_player_header->Players[j];
				if (otherStaticPlayer.PlayerID == 0 || otherStaticPlayer.CurrentBiped.index == -1 || j == i) continue;
				AObject* otherPlayer = object_table_header->Objects[otherStaticPlayer.CurrentBiped.index].Offset;
				// store vel and pos
				playerPosses[j] = otherPlayer->World;
				playerVels[j] = otherPlayer->Velocity;
				//otherPlayer->World.x += 3.0f;
				short ping = staticPlayer.Ping;
				//ping = playerInfos[i].getAveragePing();
				playerHistories[j].readDataIntoPlayer(ping + ping / 5, otherPlayer);
				//otherPlayer->Velocity.x = 0.0f;
				//otherPlayer->Velocity.y = 0.0f;
				//otherPlayer->Velocity.z = 0.0f;
				//updatePhysics(otherStaticPlayer.CurrentBiped);
				long ostcb = (long)otherStaticPlayer.CurrentBiped.ID << 16 | otherStaticPlayer.CurrentBiped.index;
				logfile << ostcb;
				__asm
				{
					pushad
						push ostcb
						call dword ptr ds : [UPDATE_PHYSICS_ADDR]
						popad
				}
			}
			//char res = oUpdateObject(id);

			// restore vel and pos
			for (short j = 0; j < 16; ++j) {
				Static_Player& otherStaticPlayer = static_player_header->Players[j];
				if (otherStaticPlayer.PlayerID == 0 || otherStaticPlayer.CurrentBiped.index == -1 || j == i) continue;
				AObject* otherPlayer = object_table_header->Objects[otherStaticPlayer.CurrentBiped.index].Offset;
				otherPlayer->World = playerPosses[j];
				//otherPlayer->Velocity = playerVels[j];
				//updatePhysics(otherStaticPlayer.CurrentBiped);
				long ostcb = (long)otherStaticPlayer.CurrentBiped.ID << 16 | otherStaticPlayer.CurrentBiped.index;
				logfile << ostcb;
				__asm
				{
					pushad
						push ostcb
						call dword ptr ds : [UPDATE_PHYSICS_ADDR]
						popad
				}
			}

			//return res;
		}
	}
	//return oUpdateObject(id);
}

/*figures out what this does and why above functions return it
(guessing) above functions call detourcreate to tell client to pretty much
run the function whenever there's time, or something, something with injection, don't care, using phasor's now.*/
/*bool entryPoint()
{
oUpdateObject = (tFuncOneObject)DetourCreate((LPVOID)UPDATE_OBJECT_ADDR, dUpdateObject, DETOUR_TYPE_JMP);
oUpdateAllObjects = (tFuncNoArgs)DetourCreate((LPVOID)UPDATE_ALL_OBJECTS_ADDR, dUpdateAllObjects, DETOUR_TYPE_JMP);
return true;
}*/