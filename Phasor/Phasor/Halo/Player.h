#pragma once

#include "../../Common/Types.h"
#include "../../Common/Streams.h"
#include "../../Common/noncopyable.h"
#include "Halo.h"
#include "Game/Objects.h"
#include "HaloStreams.h"
#include <string>
#include <memory>

namespace halo
{
	namespace afk_detection { class CAFKDetection; }
	struct s_player;

	// ------------------------------------------------------------------

	#pragma pack(push, 1)

	struct s_player_structure
	{
		WORD playerJoinCount; // 0x0000
		WORD localClient; // 0x0002 always FF FF on a dedi in Halo is 00 00 if its your player
		wchar_t playerName[11]; //0x0004 it's actually only 11 not 12
		UNKNOWN(2); // 0x001A (some index)
		UNKNOWN(4); // 0x001C only seen FF FF FF FF
		BYTE team; // 0x0020
		UNKNOWN(3); // padding?
		ident m_interactionObject; // 0x0024 ie Press E to enter THIS vehicle
		WORD interactionType; // 0x0028 8 for vehicle, 7 for weapon
		WORD interactionSpecifier; // 0x002A which seat of vehicle etc
		DWORD respawnTimer; // 0x002C in ticks
		DWORD respawnTimeGrowth; // 0x0030 in ticks
		ident object_id; // 0x0034
		ident old_object_id; // 0x0038
		UNKNOWN(2); // 0x003C (cluster_index)
		bool weapon_pickup : 1; // 0x003E 0
		UNKNOWN_BITFIELD(7); // 0x003E bits 1-7
		UNKNOWN_BITFIELD(8); // 0x003E bits 8-15 WHY DO I HAVE TO HAVE THIS AGAIN?
		//UNKNOWN(2); // 0x003E screw you phasor
		UNKNOWN(4); // 0x0040 always FF FF FF FF, never accessed
		DWORD last_bullet_time; // 0x0044 gameinfo_current_time - this = time since last shot fired (1 second = 30 ticks). Something to do with autoaim.
		s_presence_item client_stuff; // yes oxide, this is confirmed...
		WORD invis_time; // 0x0068 Time until player is no longer camouflaged. (1 sec = 30 ticks)
		UNKNOWN(2); // 0x006A something to do with invis_time.
		float speed; // 0x006C
		UNKNOWN(4); // 0x0070 (teleporter_flag_id) Index to a netgame flag in the scenario, or -1 (Always 0xFFFFFFFF?)
		UNKNOWN(4); // 0x0074 (objective_mode) (Hill = 0x22 = 34) (Juggernaut = It = 0x23 = 35) (Race = 0x16 = 22) (Ball = 0x29 = 41) (Others = -1)
		ident objective_player_id; // 0x0078 full DWORD ID of player once they interact with the objective (NOT CTF) or 0xFFFFFFFF 
		UNKNOWN(4); // 0x007C (target_player) Values (this and target_player_time) used for rendering a target player's name. (Always 0xFFFFFFFF?)
		UNKNOWN(4); // 0x0080 (target_time) Timer used to fade in the target player name.
		DWORD last_death_time; // 0x0084 gameinfo_current_time - this = time since last death. (1 sec = 30 ticks)
		ident slayer_target; // 0x0088 Target player in Slayer's Kill-in-order
		bool oddman_out : 1; // 0x008C bit 0
		UNKNOWN_BITFIELD(7); // 0x008C bits 1-7
		UNKNOWN_BITFIELD(8); // 0x008C bits 8-15 WHY DO I HAVE TO HAVE THIS AGAIN?
		UNKNOWN_BITFIELD(8); // 0x008C bits 16-23 WHY DO I HAVE TO HAVE THIS AGAIN?
		UNKNOWN_BITFIELD(8); // 0x008C bits 24-31 WHY DO I HAVE TO HAVE THIS AGAIN?
		//UNKNOWN(4); // 0x008C screw you phasor
		UNKNOWN(6); // 0x0090
		WORD killstreak; // 0x0096 How many kills the player has gotten in their lifetime.
		UNKNOWN(2); // 0x0098 (multikill) 0 on spawn, 1 when player gets kill, stays 1 until death/leave
		UNKNOWN(2); // 0x009A (last_kill_time)
		WORD kills; // 0x009C
		UNKNOWN(6);
		WORD assists; // 0x00A4
		UNKNOWN(6);
		WORD betrayals; // 0x00AC
		WORD deaths; // 0x00AE
		WORD suicides; // 0x00B0
		// cbf with the rest
		UNKNOWN(0x14e);
	};
	static_assert(sizeof(s_player_structure) == 0x0200, "bad");

	#pragma pack(pop)

	struct s_player_table
	{
		s_table_header header;
		s_player_structure players[16];
	};

	struct s_player : private noncopyable
	{
		std::string hash, ip;
		WORD port;
		int memory_id;
		bool is_admin, authenticating_hash, sv_killed, force_entered;
		s_player_structure* mem;
		std::unique_ptr<afk_detection::CAFKDetection> afk;
		std::unique_ptr<PlayerConsoleStream> console_stream;
		std::unique_ptr<PlayerChatStream> chat_stream;

		// ----------------------------------------------------------------
		explicit s_player(int memory_id);
		~s_player();

		objects::s_halo_biped* get_object() const;
		void Kick() const;
		void ChangeTeam(BYTE new_team, bool forcekill=true);
		void Kill();
		void ApplyCamo(float duration) const;
		void SetSpeed(float speed) const;
		void checkAndSetAdmin();
		bool InVehicle() const; 
		ident getPlayerIdent() const;
	};

	s_player_structure* GetPlayerMemory(int index);
	
	// -----------------------------------------------------------------

}