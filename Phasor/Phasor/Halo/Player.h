#pragma once

#include "../../Common/Types.h"
#include "../../Common/Streams.h"
#include "../../Common/noncopyable.h"
#include "Halo.h"
#include "Game/Objects.h"
#include "HaloStreams.h"
#include <string>
#include <memory>

// Oxide if you're reading this, everything I added is confirmed, and everything that is tested/unconfirmed
// from like other sources etc I used UNKNOWN(size) and described what I thought it did in the comment for that block.
// I didn't add anything I wasn't sure about, because I know you don't like clutter.

namespace halo
{
	namespace afk_detection { class CAFKDetection; }
	struct s_player;

	// ------------------------------------------------------------------

	#pragma pack(push, 1)

	union score_stats {
		struct s_ctf {
			WORD flag_steals;			// 0x00C4
			WORD flag_returns;			// 0x00C8
			WORD flag_scores;			// 0x00CC
		}ctf_stats;

		struct s_slayer {
		}slayer_stats;

		struct s_koth {
			WORD hill_time;				// 0x00C4
		}koth_stats;

		struct s_oddball {
			UNKNOWN(2);					// 0x00C4
			WORD target_kills;			// 0x00C6
			WORD kills_while_target;	// 0x00C8
		};

		struct s_race {
			WORD time;					// 0x00C4
			WORD laps;					// 0x00C6
			WORD best_time;				// 0x00C8 in ticks
		}race_stats;
	};

	struct s_player_action_keys {
		bool melee : 1;				// 0
		bool action : 1;			// 1
		UNKNOWN_BITFIELD(2);		// 2-3
		bool flashlight : 1;		// 4
		UNKNOWN_BITFIELD(3);		// 5-7
		UNKNOWN_BITFIELD(6);		// WHY DO I HAVE TO REPEAT THIS PHASOR? 8-13
		bool reload : 1;			// 14
		UNKNOWN_BITFIELD(1);		// 15
	};
	BOOST_STATIC_ASSERT(sizeof(s_player_action_keys) == 2);

	enum class e_weapon_slot : WORD {
		primary = 0,
		secondary = 1,
		tertiary = 2,
		quarternary = 3
	};

	enum class e_nade_type : WORD {
		frag = 0,
		plasma = 1
	};

	struct s_player_structure
	{
		WORD playerJoinCount;					// 0x0000
		WORD localClient;						// 0x0002 always FF FF on a dedi in Halo is 00 00 if its your player
		wchar_t playerName[12];					// 0x0004 it's actually only 11 not 12
		UNKNOWN(4);								// 0x001C only seen FF FF FF FF
		BYTE team;								// 0x0020
		UNKNOWN(3);								// 0x0021 padding?
		ident m_interactionObject;				// 0x0024 ie Press E to enter THIS vehicle
		WORD interactionType;					// 0x0028 8 for vehicle, 7 for weapon
		WORD interactionSpecifier;				// 0x002A which seat of vehicle etc
		DWORD respawnTimer;						// 0x002C in ticks
		DWORD respawnTimeGrowth;				// 0x0030 in ticks
		ident object_id;						// 0x0034
		ident old_object_id;					// 0x0038
		UNKNOWN(2);								// 0x003C (cluster_index)
		bool weapon_pickup : 1;					// 0x003E 0
		UNKNOWN_BITFIELD(7);					// 0x003E bits 1-7
		UNKNOWN_BITFIELD(8);					// 0x003E bits 8-15 WHY DO I HAVE TO REPEAT THIS PHASOR?
		UNKNOWN(4);								// 0x0040 always FF FF FF FF, never accessed
		DWORD last_bullet_time;					// 0x0044 gameinfo_current_time - this = time since last shot fired (1 second = 30 ticks). Something to do with autoaim.
		s_presence_item client_stuff;			// yes oxide, this is confirmed...
		WORD invis_time;						// 0x0068 Time until player is no longer camouflaged. (1 sec = 30 ticks)
		UNKNOWN(2);								// 0x006A something to do with invis_time.
        BYTE memoryId;
		UNKNOWN(4);
		float speed;							// 0x006C
		UNKNOWN(4);								// 0x0070 (teleporter_flag_id) Index to a netgame flag in the scenario, or -1 (Always 0xFFFFFFFF?)
		UNKNOWN(4);								// 0x0074 (objective_mode) (Hill = 0x22 = 34) (Juggernaut = It = 0x23 = 35) (Race = 0x16 = 22) (Ball = 0x29 = 41) (Others = -1)
		ident objective_player_id;				// 0x0078 full DWORD ID of player once they interact with the objective (NOT CTF) or 0xFFFFFFFF 
		UNKNOWN(4);								// 0x007C (target_player) Values (this and target_player_time) used for rendering a target player's name. (Always 0xFFFFFFFF?)
		UNKNOWN(4);								// 0x0080 (target_time) Timer used to fade in the target player name.
		DWORD last_death_time;					// 0x0084 gameinfo_current_time - this = time since last death. (1 sec = 30 ticks)
		ident slayer_target;					// 0x0088 Target player in Slayer's Kill-in-order
		bool oddman_out : 1;					// 0x008C 0
		UNKNOWN_BITFIELD(7);					// 0x008C 1-7
		UNKNOWN(3);								// 0x008D 
		UNKNOWN(6);								// 0x0090
		WORD killstreak;						// 0x0096 How many kills the player has gotten in their lifetime.
		UNKNOWN(2);								// 0x0098 (multikill) 0 on spawn, 1 when player gets kill, stays 1 until death/leave
		UNKNOWN(2);								// 0x009A (last_kill_time)
		WORD kills;								// 0x009C
		UNKNOWN(6);								// 0x009E (Padding?)
		WORD assists;							// 0x00A4
		UNKNOWN(6); // A6
		WORD betrayals;							// 0x00AC
		WORD deaths;							// 0x00AE
		WORD suicides;							// 0x00B0 Actually suicides + betrayals.
		UNKNOWN(0xE);							// 0x00B2
		WORD teamkills;							// 0x00C0
		UNKNOWN(2);								// 0x00C2 (Padding)
		score_stats gametype_stats;				// 0x00C4
		UNKNOWN(2);								// 0x00CA
		DWORD telefrag_timer;					// 0x00CC
		DWORD quit_time;						// 0x00D0
		UNKNOWN_BITFIELD(8);					// 0x00D4
		UNKNOWN_BITFIELD(8);					// 0x00D5
		UNKNOWN(6);								// 0x00D6
		DWORD ping;								// 0x00DC
		DWORD teamkill_number;					// 0x00E0
		DWORD teamkill_timer;					// 0x00E4 Time since last betrayal, in ticks.
		UNKNOWN(2);								// 0x00E8 some timer, increases once every half second until it hits 36, then restarts.
		UNKNOWN(0xE);							// 0x00EA
		vect3d location;						// 0x00F8
		UNKNOWN(4);								// 0x0104 some ident
		UNKNOWN(8);								// 0x0108
		UNKNOWN(4);								// 0x0110 some timer
		UNKNOWN(8);								// 0x0114
		s_player_action_keys action_keys;		// 0x011C
		UNKNOWN(0x1A);							// 0x011E
		float xy_aim;							// 0x0138 (Lags. Use aim in s_halo object instead)
		float z_aim;							// 0x013C (Lags. use aim in s_halo_object instead)
		float forward;							// 0x0140 Negative means backwards. Lags. (-1, -sqrt(2)/2, 0, sqrt(2)/2, 1)
		float left;								// 0x0144 Negative means right. Lags. (-1, -sqrt(2)/2, 0, sqrt(2)/2, 1)
		float rof_speed;						// 0x0148 Rate of fire speed, increases while player is shooting.
		e_weapon_slot weap_slot;				// 0x014C (Lags)
		e_nade_type nade_type;					// 0x014E (Lags)
		UNKNOWN(4);								// 0x0150 (Padding Maybe?)
		vect3d xyz_aim2;						// 0x0154 (Lags)
		UNKNOWN(0x10);							// 0x0160
		vect3d location2;						// 0x0170
		// rest are unknown
		UNKNOWN(0x84);							// 0x017C
	};
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

		objects::s_halo_unit* get_object() const;
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