#pragma once

#include "../../../Common/Types.h"
#include "Objects.h"
#include "../Player.h"

namespace halo { namespace game {

	// Structures
	// 
	#pragma pack(push, 1)
	struct chatData
	{
		DWORD type;
		DWORD player;
		wchar_t* msg;
	};
	#pragma pack(pop)

	s_player* GetPlayer(int index);
	s_player* GetPlayerFromRconId(unsigned int playerNum);
	s_player* GetPlayerFromAddress(s_player_structure* player);

	// --------------------------------------------------------------------
	// Events

	// Called when a new game starts
	void OnNewGame(const char* map); // called from Server.cpp

	// Called when a game stage ends
	void OnGameEnd(DWORD mode); // called form Server.cpp

	// Called when a player joins (after verification).
	void __stdcall OnPlayerWelcome(DWORD playerId);

	// Called when a player quits
	void __stdcall OnPlayerQuit(DWORD playerId);

	// Called when a player's team is being assigned
	DWORD __stdcall OnTeamSelection(DWORD cur_team, LPBYTE lpMachinePtr);

	// Called when a player tries to change team
	bool __stdcall OnTeamChange(DWORD playerId, DWORD team);

	// Called when a player is about to spawn (object already created)
	void __stdcall OnPlayerSpawn(DWORD playerId, DWORD m_objectId);

	// Called after the server has been notified of a player spawn
	void __stdcall OnPlayerSpawnEnd(DWORD playerId, DWORD m_objectId);

	// Called when a weapon is created
	void __stdcall OnObjectCreation(DWORD m_weaponId);

	// Called when a weapon is assigned to an object
	DWORD __stdcall OnWeaponAssignment(DWORD playerId, DWORD owningObjectId,
		objects::s_object_info* curWeapon, DWORD order);

	// Called when a player can interact with an object
	bool __stdcall OnObjectInteraction(DWORD playerId, DWORD m_ObjId);

	// Called when a player's position is updated
	void OnClientUpdate(s_player& player);

	// Called when an object's damage is being looked up
	void __stdcall OnDamageLookup(DWORD receivingObj, DWORD causingObj, LPBYTE tagEntry);

	// Called when someone chats in the server
	void __stdcall OnChat(chatData* chat);

	// Called when a player attempts to enter a vehicle
	bool __stdcall OnVehicleEntry(DWORD playerId);

	// Called when a player is being ejected from a vehicle
	bool __stdcall OnVehicleEject(objects::s_halo_biped* m_playerObject, bool forceEjected);

	// Called when a player dies
	void __stdcall OnPlayerDeath(DWORD killerId, DWORD victimId, DWORD mode);

	// Called when a player gets a double kill, spree etc
	void __stdcall OnKillMultiplier(DWORD playerId, DWORD multiplier);

	// Called when a weapon is reloaded
	bool __stdcall OnWeaponReload(DWORD m_WeaponId); 
}}