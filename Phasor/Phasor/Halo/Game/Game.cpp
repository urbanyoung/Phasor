#include "Game.h"
#include "../HaloStreams.h"
#include "../Server/MapLoader.h"
#include "../Player.h"
#include "../AFKDetection.h"
#include "../Alias.h"
#include "../../Globals.h"
#include "../../../Common/MyString.h"
#include "../../../Scripts/script-events.h"
#include "../tags.h"
#include "../Server/Chat.h"
#include "../Server/MapVote.h"
#include "../Server/Lead.h"
#include <vector>

namespace halo { namespace game {
	typedef std::unique_ptr<s_player> s_player_ptr;
	s_player_ptr PlayerList[16];

	void cleanupPlayers(bool notify_scripts)
	{
		for (int i = 0; i < 16; i++) {
			if (PlayerList[i]) {
				if (notify_scripts) scripting::events::OnPlayerLeave(*PlayerList[i]);
				PlayerList[i].reset();
			}
		}
	}

	inline bool valid_index(DWORD playerIndex)
	{
		return playerIndex < 16;
	}

	// Find the player based on their memory id.
	s_player* getPlayer(int index)
	{
		if (!valid_index(index)) return NULL;
		return PlayerList[index] ? PlayerList[index].get() : NULL;
	}

	// Find the player based on their player/rcon/machine id.
	s_player* getPlayerFromRconId(unsigned int playerNum)
	{
		for (int i = 0; i < 16; i++) {
			if (PlayerList[i] && PlayerList[i]->mem->playerNum == playerNum)
				return PlayerList[i].get();
		}
		return NULL;
	}

	s_player* getPlayerFromAddress(s_player_structure* player)
	{
		int indx = ((DWORD)player - *(DWORD*)ADDR_PLAYERBASE) / sizeof(s_player_structure);
		return getPlayer(indx);
	/*	g_PrintStream->print("indx %i", indx);
		for (int i = 0; i < 16; i++) {
			if (PlayerList[i] && PlayerList[i]->mem == player)
				return PlayerList[i].get();
		}
		return NULL;*/
	}

	s_player* getPlayerFromObject(objects::s_halo_biped* obj)
	{
		for (int i = 0; i < 16; i++) {
			if (PlayerList[i] && PlayerList[i]->get_object() == (void*)obj)
				return PlayerList[i].get();
		}
		return NULL;
	}

	s_player* getPlayerFromObjectId(ident id)
	{
		for (int i = 0; i < 16; i++) {
			if (PlayerList[i] && PlayerList[i]->mem->object_id == id)
				return PlayerList[i].get();
		}
		return NULL;
	}

	s_player* getPlayerFromHash(const std::string& hash)
	{
		for (int i = 0; i < 16; i++) {
			if (PlayerList[i] && PlayerList[i]->hash == hash)
				return PlayerList[i].get();
		}
		return NULL;
	}

	// Called when a game stage ends
	void OnGameEnd(DWORD mode)
	{
		switch (mode)
		{
		case 1: // just ended (in game scorecard is shown)
			{
				afk_detection::Disable();
                server::lead::OnGameEnd();
				g_GameLog->WriteLog(kGameEnd, L"The game has ended.");
				*g_PrintStream << "The game is ending..." << endl;

			} break;

		case 2:
			{
				objects::ClearManagedObjects();
			} break;
		}	

		scripting::events::OnGameEnd(mode);
	}

	// Called when a new game starts
	void OnNewGame(const char* map)
	{
		objects::ClearManagedObjects();
		afk_detection::Enable();
		halo::BuildTagCache();
        server::lead::OnGameStart();
		
		g_GameLog->WriteLog(kGameStart, "A new game has started on map %s", map);

		scripting::events::OnNewGame(map);
	}

	// Called when a player joins (after verification).
	void __stdcall OnPlayerWelcome(DWORD playerId)
	{
		if (!valid_index(playerId)) {
			*g_PhasorLog << "Player joined with invalid index??" << endl;
			return;
		}
		PlayerList[playerId].reset(new s_player(playerId));
		s_player* player = getPlayer(playerId);	
		g_GameLog->WriteLog(kPlayerJoin, L"%s (%s ip: %s:%i)", 
			player->mem->playerName, WidenString(player->hash).c_str(),
			WidenString(player->ip).c_str(), player->port);
		alias::OnPlayerJoin(*player);
		scripting::events::OnPlayerJoin(*player);
	}

	// Called when a player quits
	void __stdcall OnPlayerQuit(DWORD playerId)
	{
		if (!valid_index(playerId)) {
			*g_PhasorLog << "Player left with invalid index??" << endl;
			return;
		}
		s_player* player = getPlayer(playerId);

		if (player) {			
			g_GameLog->WriteLog(kPlayerLeave, L"%s (%s)", 
				player->mem->playerName, WidenString(player->hash).c_str()
				);

			scripting::events::OnPlayerLeave(*player);
			PlayerList[playerId].reset();
		}
	}

	DWORD __stdcall OnTeamSelection(DWORD cur_team, server::s_machine_info* machine)
	{
		return scripting::events::OnTeamDecision(cur_team);
	}

	/*! \todo add checks when sv_teams_change or w/e is enabled */
	bool __stdcall OnTeamChange(DWORD playerId, DWORD new_team)
	{
		s_player* player = getPlayer(playerId);
		bool allow = true;

		if (player && new_team != player->mem->team) {
			allow = scripting::events::OnTeamChange(*player, true, new_team,
				player->mem->team);
		}

		if (!allow) {
			g_GameLog->WriteLog(kPlayerChange, L"Blocked %s from changing team.", 
				player->mem->playerName);
		}

		return allow;
	}

	// Called when a player is about to spawn (object already created)
	void __stdcall OnPlayerSpawn(DWORD playerId, ident m_objectId)
	{
		halo::s_player* player = getPlayer(playerId);
		if (!player) return;
		scripting::events::OnPlayerSpawn(*player, m_objectId);
	}

	// Called after the server has been notified of a player spawn
	void __stdcall OnPlayerSpawnEnd(DWORD playerId, ident m_objectId)
	{
		halo::s_player* player = getPlayer(playerId);
		if (!player) return;
		scripting::events::OnPlayerSpawnEnd(*player, m_objectId);
	}

    // Called when an object is being destroyed
    void __stdcall OnObjectDestroy(ident m_objid)
    {
        objects::OnObjectDestroy(m_objid);
        server::lead::OnObjectDestroy(m_objid);
    }

	// Called when a weapon is created
	void __stdcall OnObjectCreation(ident m_objectId)
	{
        server::lead::OnObjectCreation(m_objectId);
		scripting::events::OnObjectCreation(m_objectId);        
	}

	bool __stdcall OnObjectCreationAttempt(s_player_structure* probably_not_a_player,
                                           objects::s_object_creation_disposition* creation_info)
	{
        // note: this argument is most definitely wrong for every single call 
        // that isn't creating a player. It probably isn't even a pointer
        // most of the time. However, getPlayerFromAddress will validate
        // it for us.
        s_player* player = getPlayerFromAddress(probably_not_a_player);

		bool allow = true;
		boost::optional<halo::ident> changeId = scripting::events::OnObjectCreationAttempt(creation_info, player, allow);

		if (!changeId) return allow;
		else {
			
            halo::s_tag_entry* change_tag = LookupTag(*changeId);
			halo::s_tag_entry* default_tag = LookupTag(creation_info->map_id);

			if (change_tag && default_tag && change_tag->tagType == default_tag->tagType) {
                creation_info->map_id = *changeId;
			}
			return true;
		}		
	}
	
	DWORD __stdcall OnWeaponAssignment(DWORD playerId, ident owningObjectId,
		s_object_info* curWeapon, DWORD order)
	{
		halo::s_player* player = game::getPlayer(playerId);
        ident weap_id = curWeapon->id;

		boost::optional<halo::ident> changeId = 
            scripting::events::OnWeaponAssignment(player, owningObjectId, order, weap_id);

		// can return 0xFFFFFFFF to not assign a weapon
        if (changeId && (!changeId->valid() || LookupTag(*changeId))) return *changeId;
		else return weap_id;	
	}

	// Called when a player can interact with an object
	bool __stdcall OnObjectInteraction(DWORD playerId, ident m_ObjId)
	{
		bool allow = true;
		halo::s_player* player = game::getPlayer(playerId);
        objects::s_halo_object* obj = (objects::s_halo_object*)
            objects::GetObjectAddress(m_ObjId);
		if (player && obj) {

			allow = scripting::events::OnObjectInteraction(*player, m_ObjId, obj->map_id);		
		}
		return allow;
	}
	
	void OnClientUpdate(s_player& player)
	{
		// scripts called from server
		player.afk->CheckPlayerActivity();
	}


	// Called when someone chats in the server
    void __stdcall OnChat(server::s_machine_info* machine, server::chat::s_chat_data* chat)
	{
		using namespace server::chat;
		static const wchar_t* typeValues[] = {L"GLOBAL", L"TEAM", L"VEHICLE"};
		s_player* sender = getPlayer(chat->player);
		if (!sender || chat->type < kChatAll || chat->type > kChatVehicle) return;
        if (machine->playerNum != sender->mem->playerNum) return;

		int length = wcslen(chat->msg);
		if (length > 256) return;

		std::wstring send_msg = chat->msg;
		
		sender->afk->MarkPlayerActive();

		std::string change_msg;
		bool allow = scripting::events::OnServerChat(sender, NarrowString(chat->msg),
			chat->type, change_msg);

		if (!allow) return;
		if (change_msg.size()) send_msg = WidenString(change_msg);
		if (!server::mapvote::OnServerChat(*sender, chat->msg)) return;

		g_GameLog->WriteLog(kPlayerChat, L"[%s] %s: %s", typeValues[chat->type], 
			sender->mem->playerName, send_msg.c_str());

		DispatchChat(chat->type, send_msg.c_str(), sender);
	}

	// Called when a player attempts to enter a vehicle
	bool __stdcall OnVehicleEntry(DWORD playerId)
	{
		s_player* player = getPlayer(playerId);
		if (!player) return true;

		objects::s_halo_biped* obj = (objects::s_halo_biped*)objects::GetObjectAddress(player->mem->object_id);
		if (!obj) return true;

		return scripting::events::OnVehicleEntry(*player, player->mem->m_interactionObject,
			player->mem->interactionSpecifier, !player->force_entered);
	}

	// Called when a player is being ejected from a vehicle
	bool __stdcall OnVehicleEject(objects::s_halo_biped* player_obj, bool forceEjected)
	{
		s_player* player = getPlayerFromObject(player_obj);
		if (!player) return true;
		return scripting::events::OnVehicleEject(*player, forceEjected);
	}

	// Called when a player dies
	bool __stdcall OnPlayerDeath(DWORD killerId, DWORD victimId, DWORD mode)
	{
		s_player* victim = getPlayer(victimId);
		s_player* killer = getPlayer(killerId);

		if (!victim) return true;

		// log the death based on type
		switch (mode)
		{
		case 1: // fall dmg or server kill
			{
				if (victim->sv_killed)	mode = 0;
	
				g_GameLog->WriteLog(kPlayerDeath, L"%s died (%i).", 
					victim->mem->playerName, mode);

			} break;

		case 2: // killed by guardians
			{
				g_GameLog->WriteLog(kPlayerDeath, L"%s was killed by the guardians.", 
					victim->mem->playerName);
			} break;

		case 3: // killed by a vehicle
			{
				g_GameLog->WriteLog(kPlayerDeath, L"%s was killed by a vehicle.", 
					victim->mem->playerName);
			} break;

		case 4: // killed by another player
			{
				if (killer) {
					g_GameLog->WriteLog(kPlayerDeath, L"%s was killed by %s.", 
						victim->mem->playerName, killer->mem->playerName);
				}
			} break;

		case 5: // betrayed
			{
				if (killer) {
					g_GameLog->WriteLog(kPlayerDeath, L"%s was betrayed by %s.", 
						victim->mem->playerName, killer->mem->playerName);
				}
			} break;

		case 6: // suicide
			{
				g_GameLog->WriteLog(kPlayerDeath, L"%s committed suicide.", 
					victim->mem->playerName);
			} break;
		}
			
		return scripting::events::OnPlayerKill(*victim, killer, mode);
	}

	// Called when a player gets a double kill, spree etc
	void __stdcall OnKillMultiplier(DWORD playerId, DWORD multiplier)
	{
		halo::s_player* player = getPlayer(playerId);
		if (!player) return;
		scripting::events::OnKillMultiplier(*player, multiplier);
	}

	// Called when a weapon is reloaded
	bool __stdcall OnWeaponReload(ident m_WeaponId)
	{
		objects::s_halo_weapon* weap = (objects::s_halo_weapon*)objects::GetObjectAddress(m_WeaponId);
		if (!weap) return true;
		halo::s_player* player = getPlayer(weap->base.ownerPlayer.slot);
		return scripting::events::OnWeaponReload(player, m_WeaponId);
	}
}}