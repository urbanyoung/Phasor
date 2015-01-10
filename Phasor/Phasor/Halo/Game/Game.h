#pragma once

#include "../../../Common/Types.h"
#include "Objects.h"
#include "../Player.h"

namespace halo { 
	struct ident;
	struct s_object_info;
	struct s_tag_entry;

	namespace server {
		struct s_machine_info;
		namespace chat {
			struct s_chat_data;
		}
	}
	
	namespace game {
		struct s_chat_data;

	s_player* getPlayer(int index);
	s_player* getPlayerFromRconId(unsigned int playerNum);
	s_player* getPlayerFromAddress(s_player_structure* player);
	s_player* getPlayerFromObject(objects::s_halo_biped* obj);
	s_player* getPlayerFromObjectId(ident id);
	s_player* getPlayerFromHash(const std::string& hash);

	void cleanupPlayers(bool notify_scripts);

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
	DWORD __stdcall OnTeamSelection(DWORD cur_team, server::s_machine_info* machine);

	// Called when a player tries to change team
	bool __stdcall OnTeamChange(DWORD playerId, DWORD team);

	// Called when a player is about to spawn (object already created)
	void __stdcall OnPlayerSpawn(DWORD playerId, ident m_objectId);

	// Called after the server has been notified of a player spawn
	void __stdcall OnPlayerSpawnEnd(DWORD playerId, ident m_objectId);

    // Called when an object is being destroyed
    void __stdcall OnObjectDestroy(ident m_objid);

	// Called when a weapon is created
	void __stdcall OnObjectCreation(ident m_objectId);

	bool __stdcall OnObjectCreationAttempt(s_player_structure* player,
                                           objects::s_object_creation_disposition* creation_info);

	// Called when a weapon is assigned to an object
	DWORD __stdcall OnWeaponAssignment(DWORD playerId, ident owningObjectId,
		s_object_info* curWeapon, DWORD order);

	// Called when a player can interact with an object
	bool __stdcall OnObjectInteraction(DWORD playerId, ident m_ObjId);

	// Called when a player's position is updated
	void OnClientUpdate(s_player& player);

	// Called when someone chats in the server
	void __stdcall OnChat(server::s_machine_info* machine, server::chat::s_chat_data* chat);

	// Called when a player attempts to enter a vehicle
	bool __stdcall OnVehicleEntry(DWORD playerId);

	// Called when a player is being ejected from a vehicle
	bool __stdcall OnVehicleEject(objects::s_halo_biped* m_playerObject, bool forceEjected);

	// Called when a player dies
	bool __stdcall OnPlayerDeath(DWORD killerId, DWORD victimId, DWORD mode);

	// Called when a player gets a double kill, spree etc
	void __stdcall OnKillMultiplier(DWORD playerId, DWORD multiplier);

	// Called when a weapon is reloaded
	bool __stdcall OnWeaponReload(ident m_WeaponId); 
}}