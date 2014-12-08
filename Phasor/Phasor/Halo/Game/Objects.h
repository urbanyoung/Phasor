#pragma once

#include "../../../Common/Types.h"
#include "../../../Common/vect.h"
#include "../Halo.h"
#include "../tags.h"
#include <boost/optional.hpp>

namespace halo {
struct s_player;
namespace objects {
#pragma pack(push, 1)

// Some structure issues were clarified thanks to the code at:
// http://code.google.com/p/halo-devkit/source/browse/trunk/halo_sdk/Engine/objects.h
//


// Oxide if you're reading this, everything I added is confirmed, and everything that is tested/unconfirmed
// from like other sources etc I used UNKNOWN(size) and described what I thought it did in the comment for that block.
// I didn't add anything I wasn't sure about, because I know you don't like clutter.

struct s_object_physics
{
	bool noCollision : 1;			//0
	bool is_on_ground : 1;			//1
	bool ignoreGravity : 1;			//2
	UNKNOWN_BITFIELD(2);			//3-4
	bool stationary : 1;			//5
	UNKNOWN_BITFIELD(1);			//6
	bool noCollision2 : 1;			//7 has no collision box, projectiles etc pass right through
	UNKNOWN_BITFIELD(3);			//8-10
	bool connected_to_map : 1;		//11 always true?
	UNKNOWN_BITFIELD(4);			//12-15
	bool garbage : 1;				//16
	UNKNOWN_BITFIELD(1);			//17
	bool does_not_cast_shadow : 1;	//18
	UNKNOWN_BITFIELD(2);			//19-20
	bool outside_of_map : 1;		//21
	bool beautify : 1;				//22 always false?
	UNKNOWN_BITFIELD(1);			//23
	bool collision : 1;				//24 opposite of noCollision(2)
	UNKNOWN_BITFIELD(7);			//25-31
};
BOOST_STATIC_ASSERT(sizeof(s_object_physics) == 0x4);

// thanks Wizard
// you are welcome
enum class e_object_type : WORD {
    biped = 0,
    vehicle = 1,
    weapon = 2,
    equipment = 3,
    garbage = 4,
    projectile = 5,
    scenery = 6,
    machine = 7,
    control = 8,
    light = 9,
    placeholder = 10,
    sound = 11
};

struct s_damage_flags
{
	// guessing they're always false because they apply to AI
	bool body_damage_effect_applied : 1;		//0 Always False?
	bool shield_damage_effect_applied : 1;		//1 Always False?
	bool body_health_empty : 1;					//2 Always False?
	bool shield_empty : 1;						//3 Always False?
	bool kill : 1;								//4 Always False?
	bool silent_kill : 1;						//5 Always False?
	bool damage_berserk : 1;					//6 Always False? (actor berserk related)
	UNKNOWN_BITFIELD(1);						//7
	bool immune_to_dmg : 1;						//8 according to oxide
	UNKNOWN_BITFIELD(2);						//9-10
	bool cannot_take_damage : 1;				//11
	bool shield_recharging : 1;					//12
	bool killed_no_stats : 1;					//13 Always False?
	UNKNOWN_BITFIELD(2);						//14-15
};
BOOST_STATIC_ASSERT(sizeof(s_damage_flags) == 2);

//pretty much directly from OpenSource
//**//**
enum e_attachment_type : BYTE
{
	_attachment_type_invalid = 0xFF,
	_attachment_type_light = 0,
	_attachment_type_looping_sound,
	_attachment_type_effect,
	_attachment_type_contrail,
	_attachment_type_particle,
	_attachment_type,
};

struct s_attachments_data
{
	e_attachment_type attached_types[8];		// 0x144
	// game state datum_index
	// ie, if Attachments[x]'s definition (object_attachment_block[x]) says it is a 'cont'
	// then the datum_index is a contrail_data handle
	ident attachment_indices[8];			// 0x14C
	ident first_widget_index;				// 0x16C
};
BOOST_STATIC_ASSERT(sizeof(s_attachments_data) == 0x2C);

struct s_object_header_block_reference
{
	WORD size;
	WORD offset;
};
BOOST_STATIC_ASSERT(sizeof(s_object_header_block_reference) == 4);

struct s_halo_object // generic object header
    {
    ident map_id;						// 0x0000
    UNKNOWN(8);							// 0x0004
	unsigned long existance_time;		// 0x000C
	s_object_physics physics;			// 0x0010
    unsigned long timer;				// 0x0014 continually counts up from like 89000...
    UNKNOWN(0x44);						// 0x0018
    vect3d location;					// 0x005C
    vect3d velocity;					// 0x0068
    vect3d rotation;					// 0x0074
    vect3d scale;						// 0x0080
    vect3d rotationalVel;				// 0x008C
	unsigned long locationId;			// 0x0098
    UNKNOWN(4);							// 0x009C padding?
	vect3d center;						// 0x00A0 very close to location, but not quite?
	float radius;						// 0x00AC
	float scaling;						// 0x00B0 Seems to be some random float for all objects (all same objects have same value)
    e_object_type objectType;			// 0x00B4
	UNKNOWN(2);							// 0x00B6 padding
	WORD game_objective;				// 0x00B8 if objective then this >= 0, -1 = not game object. Otherwise: (Red = 0, Blue = 1)
	WORD namelist_index;				// 0x00BA
	UNKNOWN(0x04);						// 0x00BC moving_time, region_permutation_variant_id
    ident ownerPlayer;					// 0x00C0 (index of owner (if has one))
    ident ownerObject;					// 0x00C4 (object id of owner, if projectile is player id)
	UNKNOWN(4);							// 0x00C8 padding
	ident antr_meta_id;					// 0x00CC aka animation_index
	WORD animation_state;				// 0x00D0
	WORD time_since_animation_change;	// 0x00D2
	UNKNOWN(4);							// 0x00D4
	float max_health;					// 0x00D8
	float max_shields;					// 0x00DC
    float health;						// 0x00E0
    float shield;						// 0x00E4
	float current_shield_dmg;			// 0x00E8
	float current_body_dmg;				// 0x00EC
	UNKNOWN(4);							// 0x00F0
	float last_shield_dmg_amount;		// 0x00F4
	float last_body_dmg_amount;			// 0x00F8
	// Oxide I don't know what you were smoking when you wrote this:
	//vect3d location1;					// 0x00F8 set when in a vehicle unlike other one. best not
										// to use tho (isnt always set)
	unsigned long last_shield_dmg_time;	// 0x00FC
	unsigned long last_body_dmg_time;	// 0x0100
	WORD shields_recharge_time;			// 0x0104
	s_damage_flags dmg_flags;			// 0x0106
	UNKNOWN(4);							// 0x0108
	ident cluster_partition_index;		// 0x010C
	ident some_obj_id;					// 0x0110 object index, garbage related
    ident veh_weaponId;					// 0x0114
    ident player_curWeapon;				// 0x0118
    ident vehicleId;					// 0x011C
    BYTE bGunner;						// 0x0120
    short unkShort;						// 0x0121
    BYTE bFlashlight;					// 0x0123
	// functions
	float shields_hit;					// 0x0124 counts down from 1 after shields are hit (0 to 1)
    float shields_target;				// 0x0128 when you have an overshield it stays at 1 which is why I think the overshield drains
    float flashlightCharge;				// 0x012C intensity of flashlight as you turn it on/off.
	float assaultrifle_func;			// 0x0130 assault rifle is the only one that does something with this.
	UNKNOWN(4);							// 0x0134
	float flashlightCharge1;			// 0x0138
	float shields_hit1;					// 0x013C
	UNKNOWN(4);							// 0x0140
	s_attachments_data attachments;		// 0x0144
	UNKNOWN(4);							// 0x0170 cached_render_state_index
	UNKNOWN(2);							// 0x0174
	UNKNOWN(2);							// 0x0176 shader_permutation, shader's bitmap block index
	BYTE region_vitality[8];			// 0x0178
	BYTE region_permutation_indices[8]; // 0x0180
	real_rgb_color change_colors[4];	// 0x0188
	real_rgb_color change_colors2[4];	// 0x01B8

	// one of these are for interpolating
	s_object_header_block_reference node_orientations;								// 0x1E8 real_orientation3d[node_count]
	s_object_header_block_reference node_orientations2;								// 0x1EC real_orientation3d[node_count]
	s_object_header_block_reference node_matrix_block;								// 0x1F0 real_matrix4x3[node_count]

    //	immune to backtap: 0xB4 (boolean) what? 0xB4 is object_type...
    //	0x107 | 8 == immune to all damage
};
BOOST_STATIC_ASSERT(sizeof(s_halo_object) == 0x1F4);

struct s_unit_noninstant_actions
{
	UNKNOWN_BITFIELD(4);				//0-3
	bool is_invisible : 1;				//4
	bool powerup_additional : 1;		//5
	bool currently_controllable : 1;	//6
	UNKNOWN_BITFIELD(1);				//7
	UNKNOWN(1);							//8-15
	bool doesNotAllowVehicleEntry : 1;	//16
	UNKNOWN_BITFIELD(2);				//17-18
	bool flashlight : 1;				//19
	bool doesnt_drop_items : 1;			//20
	UNKNOWN_BITFIELD(1);				//21
	bool can_blink : 1;					//22
	UNKNOWN_BITFIELD(1);				//23
	bool is_suspended : 1;				//24
	UNKNOWN_BITFIELD(2);				//25-26
	bool is_possessed : 1;				//27
	bool flashlight_currently_on : 1;	//28 wrong?
	bool flashlight_currently_off : 1;	//29 wrong?
	UNKNOWN_BITFIELD(2);				//30-31
};
BOOST_STATIC_ASSERT(sizeof(s_unit_noninstant_actions) == 4);

// these are action flags, basically client button presses
// these don't actually control whether or not an event occurs
struct s_unit_instant_actions
{
	bool crouching : 1;				// 0
	bool jumping : 1;				// 1
	UNKNOWN_BITFIELD(2);
	bool flashlight : 1;			// 4
	UNKNOWN_BITFIELD(1);
	bool actionPress : 1;			// 6 think this is just when they initially press
									// the action button
	bool melee : 1;					// 7
	UNKNOWN_BITFIELD(2);
	bool reload : 1;				// 10
	bool primaryWeaponFire : 1;		// 11 right mouse
	bool secondaryWeaponFire : 1;  // 12 left mouse
	bool secondaryWeaponFire1 : 1; // 13
	bool actionHold : 1;           // 14 holding action button
	UNKNOWN_BITFIELD(1);
};
BOOST_STATIC_ASSERT(sizeof(s_unit_instant_actions) == 2);

enum e_unit_animation_state : BYTE
{
	_unit_animation_state_invalid = 0xFF,
	_unit_animation_state_idle = 0,
	_unit_animation_state_gesture,
	_unit_animation_state_turn_left,
	_unit_animation_state_turn_right,
	_unit_animation_state_move_front,
	_unit_animation_state_move_back,
	_unit_animation_state_move_left,
	_unit_animation_state_move_right,
	_unit_animation_state_stunned_front,
	_unit_animation_state_stunned_back,
	_unit_animation_state_stunned_left,
	_unit_animation_state_stunned_right,
	_unit_animation_state_slide_front,
	_unit_animation_state_slide_back,
	_unit_animation_state_slide_left,
	_unit_animation_state_slide_right,
	_unit_animation_state_ready,
	_unit_animation_state_put_away,
	_unit_animation_state_aim_still,
	_unit_animation_state_aim_move,
	_unit_animation_state_airborne,
	_unit_animation_state_land_soft,
	_unit_animation_state_land_hard,
	_unit_animation_state_unknown23,
	_unit_animation_state_airborne_dead,
	_unit_animation_state_landing_dead,
	_unit_animation_state_seat_enter,
	_unit_animation_state_seat_exit,
	_unit_animation_state_custom_animation,
	_unit_animation_state_impulse,
	_unit_animation_state_melee,
	_unit_animation_state_melee_airborne,
	_unit_animation_state_melee_continuous,
	_unit_animation_state_throw_grenade,
	_unit_animation_state_resurrect_front,
	_unit_animation_state_resurrect_back,
	_unit_animation_state_feeding,
	_unit_animation_state_surprise_front,
	_unit_animation_state_surprise_back,
	_unit_animation_state_leap_start,
	_unit_animation_state_leap_airborne,
	_unit_animation_state_leap_melee,
	_unit_animation_state_unknown42,		// unused AFAICT
	_unit_animation_state_berserk,

	_unit_animation_state
};

enum e_weapon_type : char
{
	rocket,
	plasma_pistol,
	shotgun,
	plasma_rifle,
	meh,
};

struct s_animation_state
{
	WORD animation_index;
	WORD frame_index;
};
BOOST_STATIC_ASSERT(sizeof(s_animation_state) == 4);

enum e_weapon_slot : short
{
	primary,
	secondary,
	tertiary,
	quarternary
};

enum e_ai_communication_type : WORD
{
	_ai_communication_type_death,
	_ai_communication_type_killing_spree,
	_ai_communication_type_hurt,
	_ai_communication_type_damage,
	_ai_communication_type_sighted_enemy,
	_ai_communication_type_found_enemy,
	_ai_communication_type_unexpected_enemy,
	_ai_communication_type_found_dead_friend,
	_ai_communication_type_allegiance_changed,
	_ai_communication_type_grenade_throwing,
	_ai_communication_type_grenade_startle,
	_ai_communication_type_grenade_sighted,
	_ai_communication_type_grenade_danger,
	_ai_communication_type_lost_contact,
	_ai_communication_type_blocked,
	_ai_communication_type_alert_noncombat,
	_ai_communication_type_search_start,
	_ai_communication_type_search_query,
	_ai_communication_type_search_report,
	_ai_communication_type_search_abandon,
	_ai_communication_type_search_group_abandon,
	_ai_communication_type_uncover_start,
	_ai_communication_type_advance,
	_ai_communication_type_retreat,
	_ai_communication_type_cover,
	_ai_communication_type_sighted_friend_player,
	_ai_communication_type_shooting,
	_ai_communication_type_shooting_vehicle,
	_ai_communication_type_shooting_berserk,
	_ai_communication_type_shooting_group,
	_ai_communication_type_shooting_traitor,
	_ai_communication_type_flee,
	_ai_communication_type_flee_leader_died,
	_ai_communication_type_flee_idle,
	_ai_communication_type_attempted_flee,
	_ai_communication_type_hiding_finished,
	_ai_communication_type_vehicle_entry,
	_ai_communication_type_vehicle_exit,
	_ai_communication_type_vehicle_woohoo,
	_ai_communication_type_vehicle_scared,
	_ai_communication_type_vehicle_falling,
	_ai_communication_type_surprise,
	_ai_communication_type_berserk,
	_ai_communication_type_melee,
	_ai_communication_type_dive,
	_ai_communication_type_uncover_exclamation,
	_ai_communication_type_falling,
	_ai_communication_type_leap,
	_ai_communication_type_postcombat_alone,
	_ai_communication_type_postcombat_unscathed,
	_ai_communication_type_postcombat_wounded,
	_ai_communication_type_postcombat_massacre,
	_ai_communication_type_postcombat_triumph,
	_ai_communication_type_postcombat_check_enemy,
	_ai_communication_type_postcombat_check_friend,
	_ai_communication_type_postcombat_shoot_corpse,
	_ai_communication_type_postcombat_celebrate,

	_ai_communication_type,
};

struct s_ai_communication_packet
{
	UNKNOWN(4);									// 0x00
	UNKNOWN(4);									// 0x04
	e_ai_communication_type dialogue_type_index;	// 0x06
	UNKNOWN(2);									// 0x08
	UNKNOWN(2);									// 0x0A ?

	UNKNOWN(2);									// 0x0C
	UNKNOWN(2);									// 0x0E ?
	UNKNOWN(2);									// 0x10 ?
	UNKNOWN(2);									// 0x14
	UNKNOWN(2);									// 0x16 ?
	UNKNOWN(2);									// 0x18
	UNKNOWN(2);									// 0x1A
	bool broken;								// 0x1C false = reformed
	UNKNOWN(3);									// 0x1D padding
};
BOOST_STATIC_ASSERT(sizeof(s_ai_communication_packet) == 0x20);

enum e_unit_speech_priority : WORD
{
	_unit_speech_none,
	_unit_speech_idle,
	_unit_speech_pain,
	_unit_speech_talk,
	_unit_speech_communicate,
	_unit_speech_shout,
	_unit_speech_script,
	_unit_speech_involuntary,
	_unit_speech_exclaim,
	_unit_speech_scream,
	_unit_speech_death,

	k_number_of_unit_speech_priorities, // NUMBER_OF_UNIT_SPEECH_PRIORITIES
};

enum e_unit_scream_type : WORD
{
	_unit_scream_type_fear,
	_unit_scream_type_enemy_grenade, // _dialogue_vocalization_hurt_enemy_grenade
	_unit_scream_type_pain,
	_unit_scream_type_maimed_limb,
	_unit_scream_type_mained_head,
	_unit_scream_type_resurrection,

	k_number_of_unit_scream_types, // NUMBER_OF_UNIT_SCREAM_TYPES
};

struct s_unit_speech
{
	e_unit_speech_priority priority;						// 0x00
	e_unit_scream_type scream;							// 0x02
	ident sound_definition_index;						// 0x04
	UNKNOWN(2);											// 0x08 time related
	UNKNOWN(2);											// 0x0A padding
	UNKNOWN(4);											// 0x0C haven't verified what is here yet
	s_ai_communication_packet ai_information;			// 0x10
};
BOOST_STATIC_ASSERT(sizeof(s_unit_speech) == 0x30);

struct s_recent_damage
{
	DWORD game_tick;				// the last game tick damage was dealt
	float damage;					// total (read: additive) damage the responsible object has done
	ident responsible_unit;
	ident responsible_player;		// would be -1 if killed by AI
};
BOOST_STATIC_ASSERT(sizeof(s_recent_damage) == 0x10);

struct s_unit_control_data
{
	UNKNOWN(2);										// 0x478
	s_unit_instant_actions actionFlags2;			// 0x47A
	WORD weap_slot;									// 0x47C
	WORD nade_type;									// 0x47E
	WORD zoomLevel;									// 0x480
	UNKNOWN(2);										// 0x482 padding
	vect3d throttle;								// 0x484
	float primary_trigger;							// 0x490
	vect3d facing_vector;							// 0x494
	vect3d aiming_vector;							// 0x4A0
	vect3d looking_vector;							// 0x4AC
};
BOOST_STATIC_ASSERT(sizeof(s_unit_control_data) == 0x40);

struct s_halo_unit {
	s_halo_object base;
	UNKNOWN(0x10);									// 0x1F4 actor related
	s_unit_noninstant_actions noninstant_actions;	// 0x204
	s_unit_instant_actions actionFlags;				// 0x208
	UNKNOWN(4);										// 0x20A
	UNKNOWN(1);										// 0x20E char shield_sapping
	BYTE base_seat_index;							// 0x20F
	UNKNOWN(4);										// 0x210 time_remaining
	UNKNOWN(4);										// 0x214 bitmask32 here
	ident player_id;								// 0x218
	WORD ai_effect_type;							// 0x21C ai_unit_effect
	WORD emotion_animation_index;					// 0x21E
	unsigned long last_bullet_time;					// 0x220
	vect3d facing;									// 0x224
	vect3d desiredAim;								// 0x230
	vect3d aim;										// 0x23C
	vect3d aimVel;									// 0x248
	vect3d aim2;									// 0x254
	vect3d aim3;									// 0x260
	UNKNOWN(sizeof(vect3d));						// 0x26C aimVel2
	vect3d directions;								// 0x278 forward, left, up, negative means other way.
	float shooting;									// 0x284 Shooting = 1, not shooting = 0
	UNKNOWN(2);										// 0x288 melee related, state enum and counter?
	BYTE time_until_flaming_death;					// 0x28A
	UNKNOWN(1);										// 0x28B looks like the amount of frames left for the ping animation. Also set to the same PersistentControlTicks value when an actor dies and they fire-wildly
	WORD throwing_grenade_state;					// 0x28C
	UNKNOWN(4);										// 0x28E
	UNKNOWN(2);										// 0x292 padding
	ident thrown_grenade_obj_id;					// 0x294
	UNKNOWN(2);										// 0x298 bitmask16 here
	WORD action;									// 0x29A something to do with actions (crouching, throwing nade, walking) (animation index, weapon type)
	UNKNOWN(4);										// 0x29C 1st word animation_index, 2nd word is initialized but unused
	BYTE bodyState;									// 0x2A0 (Standing = 4) (Crouching = 3) (Vehicle = 0)
	BYTE weap_slot;									// 0x2A1 (0 = Primary) (1 = Secondary) (2 = Tertiary) (3 = Quarternary)
	e_weapon_type weap_type;						// 0x2A2
	e_unit_animation_state animation_state;			// 0x2A3
	BYTE reloadmelee;								// 0x2A4 (5 = reloading, 7 = melee)
	BYTE shooting2;									// 0x2A5 (1 = shooting, 0 = not shooting)
	e_unit_animation_state animation_state2;			// 0x2A6
	BYTE bodyState1;								// 0x2A7 (2 = standing, 3 = crouching, 0xFF = invalid, like
													// in vehicle)
	UNKNOWN(2);										// 0x2A8
	s_animation_state PingState;					// 0x2AA
	UNKNOWN(4);										// 0x2AE
	s_animation_state FpWeaponState;				// 0x2B2
	UNKNOWN(2);										// 0x2B6 look/aim related
	real_rectangle2d lookBounds;					// 0x2B8
	real_rectangle2d aimBounds;						// 0x2C8
	UNKNOWN(0x18);									// 0x2D8
	WORD vehi_seat;									// 0x2F0
	e_weapon_slot weap_slot2;						// 0x2F2
	e_weapon_slot next_weap_slot;					// 0x2F4
	UNKNOWN(2);										// 0x2F6
	ident weaponObjId[4];							// 0x2F8
	DWORD weapLastUse[4];							// 0x308
	UNKNOWN(4);										// 0x318 seems to increase everytime you interact with the objective.
	WORD current_nade_type;							// 0x31C
	BYTE frag_nade_count;							// 0x31E
	BYTE plasma_nade_count;							// 0x31F
	BYTE zoomLevel;									// 0x320 (0xFF - no zoom, 0 - 1 zoom, 1 - 2 zoom etc)
	BYTE desiredZoomLevel;							// 0x321
	char vehicle_speech_timer;						// 0x322 counts up from 0 after reloading, shooting, or throwing a nade.
	BYTE aiming_change;								// 0x323 I don't know what units or what scale this is stored in.
	ident masterObjId;								// 0x324 Object ID controlling this s_halo_unit.
	ident masterOfWeaponsObjId;						// 0x328 Object ID controlling this s_halo_unit's weapons.
	ident passengerObjId;							// 0x32C seems to be the passenger for vehicles, 0xFFFFFFFF for bipeds.
	DWORD time_abandoned_parent;					// 0x330 in ticks
	ident someObjId;								// 0x334
	float vehicleEntryScale;						// 0x338 intensity of vehicle entry as you enter a vehicle (0 to 1)
	UNKNOWN(sizeof(float));							// 0x33C seems to be related to masterOfWeaponsObjId.
	float flashlightCharge;							// 0x340
	float flashlightLeft;							// 0x344 (0 to 1)
	float nightvisionScale;							// 0x348
	UNKNOWN(sizeof(float) * 12);					// 0x34C seat related
	float invisScale;								// 0x37C (0 to 1)
	UNKNOWN(4);										// 0x380 full_spectrum_vision_scale

	//straight from OS
	ident dialogue_definition_index;				// 0x384
	struct {
		s_unit_speech current;						// 0x388
		s_unit_speech next;							// 0x3B8 not *positive* of this field
		UNKNOWN(2);									// 0x3E8
		UNKNOWN(2);									// 0x3EA
		UNKNOWN(2);									// 0x3EC
		UNKNOWN(2);									// 0x3EE
		UNKNOWN(4);									// 0x3F0 time related
		UNKNOWN(sizeof(bool)*3);					// 0x3F4
		UNKNOWN(1);									// 0x3F7 padding
		UNKNOWN(2);									// 0x3F8
		UNKNOWN(2);									// 0x3FA
		UNKNOWN(2);									// 0x3FC
		UNKNOWN(2);									// 0x3FE
		UNKNOWN(4);									// 0x400
	}speech;
	struct {
		UNKNOWN(sizeof(WORD));						// 0x404 enum
		UNKNOWN(sizeof(WORD));						// 0x406
		UNKNOWN(sizeof(float));						// 0x408
		ident responsibleUnitObjId;					// 0x40C
	}damage;
	UNKNOWN(4);										// 0x410 flamerCausingObjId
	UNKNOWN(8);										// 0x414 padding
	UNKNOWN(4);										// 0x41C death_time
	UNKNOWN(2);										// 0x420 feign_death_timer
	WORD camo_regrowth;								// 0x422 (1 = Camo Failing due to damage/shooting)
	UNKNOWN(sizeof(float));							// 0x424 stun_amount
	UNKNOWN(2);										// 0x428 stun_timer
	WORD killstreak;								// 0x42A same as s_player_structure::killstreak
	DWORD last_kill_time;							// 0x42C game_time
	s_recent_damage recent_damage[4];				// 0x430 from most to least recent
	UNKNOWN(4);										// 0x470 padding
	BYTE shooting3;									// 0x474
	UNKNOWN(1);										// 0x475 'unused'
	UNKNOWN(2);										// 0x476 padding
	s_unit_control_data control_data;				// 0x478
	UNKNOWN(1);										// 0x4B8 last_completed_client_update_id_valid
	UNKNOWN(3);										// 0x4B9 padding
	UNKNOWN(4);										// 0x4BC last_completed_client_update_id
	UNKNOWN(0xC);									// 0x4C0 'unused'

	// The rest of bipd is composed of other tags, such as
	// mod2\characters\cyborg\cyborg
	// with these tags its size is 0xDEC
};
BOOST_STATIC_ASSERT(sizeof(s_halo_unit) == 0x4CC);

enum e_movement_state : char
{
	invalid = -1,
	standing,
	walking,
	idle_or_turning,
	gestering
};

struct s_biped_network_data
{
	BYTE grenade_counts[2];								// 0x00
	UNKNOWN(2);											// 0x02 padding
	float body_vitality;								// 0x04
	float shield_vitality;								// 0x08
	bool shield_stun_ticks_greater_than_zero;			// 0x0C
	UNKNOWN(3);											// 0x0D padding
};
BOOST_STATIC_ASSERT(sizeof(s_biped_network_data) == 0x10);

//neat I get to make my own struct
struct s_body_part
{
	UNKNOWN(sizeof(float) * 10);	// probably rotations
	vect3d location;
};
BOOST_STATIC_ASSERT(sizeof(s_body_part) == 0x34);

// Biped Struct. Definition is a two legged creature, but applies to ALL AI and all players.
struct s_halo_biped {
	bool isAirbourne : 1;							// 0x4CC 0 (from jumping/falling, not when flying in vehicle)
	UNKNOWN_BITFIELD(7);							// 0x4CC 1-7
	UNKNOWN(3);										// 0x4CD 8-31
	UNKNOWN(sizeof(BYTE) * 2);						// 0x4D0
	BYTE movement_state;							// 0x4D2
	UNKNOWN(5);										// 0x4D3 padding maybe?
	UNKNOWN(1);										// 0x4D4 something to do with walking and jumping
	UNKNOWN(1);										// 0x4D5 padding maybe?
	UNKNOWN(1);										// 0x4DA something to do with walking and jumping
	UNKNOWN(5);										// 0x4DB
	vect3d location;								// 0x4E0
	UNKNOWN(0x10);									// 0x4EC
	ident bumpedObjId;								// 0x4FC object ID of any object you bump into (rocks, weapons, players, vehicles, etc)
	char time_since_last_bump;						// 0x500 counts backwards from 0 to -15 when bumped. Glitchy, don't rely on it.
	char airborne_time;								// 0x501 in ticks
	UNKNOWN(1);										// 0x502 char slipping_time;
	UNKNOWN(1);										// 0x503 char
	char jump_time;									// 0x504 in ticks
	UNKNOWN(2);										// 0x505 char timer, melee related
	UNKNOWN(1);										// 0x507 padding
	UNKNOWN(2);										// 0x508 WORD
	UNKNOWN(2);										// 0x50A padding
	float crouch_percent;							// 0x50C
	UNKNOWN(sizeof(float));							// 0x510
	UNKNOWN(sizeof(plane3d));						// 0x514 physics related (xyzd)
	UNKNOWN(2);										// 0x524 char
	UNKNOWN(1);										// 0x526 bool baseline_valid
	UNKNOWN(2);										// 0x527 char baseline/message index
	UNKNOWN(3);										// 0x529 padding
	s_biped_network_data update_baseline;			// 0x52C
	UNKNOWN(1);										// 0x53C bool, delta_valid, only written to, never read, 'unused'
	UNKNOWN(3);										// 0x53D padding
	s_biped_network_data update_delta;				// 0x540 seems to be wrong, but I'm leaving this in.
	s_body_part bodyPart[18];						// 0x550
};
BOOST_STATIC_ASSERT(sizeof(s_halo_biped) == 0x42C);

struct s_halo_weapon {
    s_halo_object base;
    UNKNOWN(0xC2);  // 0x1F4
    WORD ammo_pack; // 0x2B6 reserve ammo
    WORD ammo_clip; // 0x2B8
    UNKNOWN(0x96);

    void SetAmmo(WORD pack, WORD clip);
    static void SyncAmmo(ident weaponId);

    // rest of weap is composed on other tags, with these tags
    // its size is 0x684
};
BOOST_STATIC_ASSERT(sizeof(s_halo_weapon) == 0x350);

struct s_halo_vehicle {
    s_halo_object base;
    UNKNOWN(0x3b8);
    DWORD idle_timer;
    UNKNOWN(0x010);
    // rest of vehi is composed of other tags, with these tags
    // its size is 0xE00
};
BOOST_STATIC_ASSERT(sizeof(s_halo_vehicle) == 0x5c0);

struct s_object_creation_disposition {
    ident map_id;
    UNKNOWN(4);
    ident player_ident;
    ident parent;
    UNKNOWN(8);
    vect3d pos;
    // rest unknown..
};
#pragma pack(pop)

struct view_vector {
    vect3d pos;
    vect3d dir;
};

void* GetObjectAddress(ident objectId);
bool DestroyObject(ident objid);

void ClearManagedObjects();

bool CreateObject(s_tag_entry* tag, ident parentId,
                  boost::optional<int> respawnTime, bool bRecycle,
                  const vect3d* location, ident& out_objid);

bool AssignPlayerWeapon(s_player& player, ident weaponid);

// Forces a player into a vehicle
// Seat numbers: 0 (driver) 1 (passenger) 2 (gunner)
bool EnterVehicle(s_player& player, ident m_vehicleId, DWORD seat);

// Forces a player to exit a vehicle
bool ExitVehicle(s_player& player);

void MoveObject(s_halo_object& object, const vect3d& pos);

// Finds an intersection between view and the environment.
// Returns true if object intersected, false otherwise.
bool FindIntersection(const view_vector& view, const halo::ident& ignore_obj,
                      vect3d& hit_pos, ident& hit_obj);

// --------------------------------------------------------------------
// Events

// Called when an object is being checked to see if it should respawn
int __stdcall VehicleRespawnCheck(ident m_objId, s_halo_vehicle* obj);

// This is called when weapons/equipment are going to be destroyed.
// todo: check ticks should be signed
bool __stdcall EquipmentDestroyCheck(int checkTicks, ident m_objId,
                                     s_halo_object* obj);

// Called when an object is being destroyed
void __stdcall OnObjectDestroy(ident m_objid);
}
}