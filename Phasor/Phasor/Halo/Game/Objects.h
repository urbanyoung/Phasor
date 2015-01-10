#pragma once

#include "../../../Common/Types.h"
#include "../../../Common/vect3d.h"
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

// thanks Wizard
enum class e_object_type : BYTE {
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

struct s_halo_object // generic object header
    {
    ident map_id; // 0x0000
    UNKNOWN(12);
    UNKNOWN_BITFIELD(2);
    bool ignoreGravity : 1;
    UNKNOWN_BITFIELD(2);
    bool stationary : 1;
    UNKNOWN_BITFIELD(1);
    bool
    noCollision : 1; // has no collision box, projectiles etc pass right through
    UNKNOWN(3);
    unsigned long timer;      // 0x0014
    UNKNOWN(0x44);            // 0x0018
    vect3d location;          // 0x005c
    vect3d velocity;          // 0x0068
    vect3d rotation;          // 0x0074
    vect3d someVector;        // 0x0080
    UNKNOWN(sizeof(vect3d));  // 0x008c
    UNKNOWN(0x1C);            // 0x98
    e_object_type objectType; // 0xB4
    UNKNOWN(0x0B);            // 0xB5
    ident ownerPlayer;        // 0x00c0 (index of owner (if has one))
    ident
    ownerObject; // 0x00c4 (object id of owner, if projectile is player id)
    UNKNOWN(0x18);
    float health; // 0x00e0
    float shield; // 0x00e4
    UNKNOWN(0x10);
    vect3d location1; // 0x00f8 set when in a vehicle unlike other one. best not
                      // to use tho (isnt always set)
    UNKNOWN(0x10);
    ident veh_weaponId;     // 0x0114
    ident player_curWeapon; // 0x0118
    ident vehicleId;        // 0x011C
    BYTE bGunner;           // 0x0120
    short unkShort;         // 0x0121
    BYTE bFlashlight;       // 0x0123
    UNKNOWN(4);
    float shield1;          // 0x0128 (same as other shield)
    float flashlightCharge; // 0x012C (1.0 when on)
    UNKNOWN(4);
    float flashlightCharge1; // 0x0134
    UNKNOWN(0xBC);

    // immune to backtap: 0xB4 (boolean)
    //	0x107 | 8 == immune to all damage
};
static_assert(sizeof(s_halo_object) == 0x1f4, "bad");

struct s_halo_biped {
    s_halo_object base;
    UNKNOWN(0x10);  // 0x1f4
    long invisible; // 0x204 (0x41 inactive, 0x51 active. probably bitfield but
                    // never seen anything else referenced)

    struct // 0x208
        {
        // these are action flags, basically client button presses
        // these don't actually control whether or not an event occurs

        bool crouching : 1; // 0
        bool jumping : 1;   // 1
        UNKNOWN_BITFIELD(2);
        bool flashlight : 1; // 4
        UNKNOWN_BITFIELD(1);
        bool actionPress : 1; // 6 think this is just when they initially press
                              // the action button
        bool melee : 1;       // 7
        UNKNOWN_BITFIELD(2);
        bool reload : 1;               // 10
        bool primaryWeaponFire : 1;    // 11 right mouse
        bool secondaryWeaponFire : 1;  // 12 left mouse
        bool secondaryWeaponFire1 : 1; // 13
        bool actionHold : 1;           // 14 holding action button
        UNKNOWN_BITFIELD(1);
    } actionFlags;
    UNKNOWN(0x26);     // 0x020A
    vect3d cameraView; // 230
    UNKNOWN(0x68);     // 0x23c
    BYTE bodyState; // 0x2A4 (2 = standing, 3 = crouching, 0xff = invalid, like
                    // in vehicle)
    UNKNOWN(0x53);  // 0x2A5
    ident primaryWeaponId;   // 0x2F8
    ident secondaryWeaponId; // 0x2FC
    ident tertiaryWeaponId;  // 0x300
    ident quartaryWeaponId;  // 0x304
    UNKNOWN(0x16);           // 0x308
    BYTE frag_nade_count;    // 0x31e
    BYTE plasma_nade_count; // 0x31f
    BYTE zoomLevel;  // 0x320 (0xff - no zoom, 0 - 1 zoom, 1 - 2 zoom etc)
    BYTE zoomLevel1; // 0x321
    UNKNOWN(0x1AA);  // 0x322
    BYTE
    isAirbourne;   // 0x4CC (from jumping/falling, not when flying in vehicle)
    UNKNOWN(0x33); // 0x4cd - 0x500
    UNKNOWN(0x0c);
    float crouch_percent;

    // The rest of bipd is composed of other tags, such as
    // mod2\characters\cyborg\cyborg
    // with these tags its size is 0xDEC
    UNKNOWN(0xDEC - 0x510);
};
static_assert(sizeof(s_halo_biped) == 0xDEC, "bad");

struct s_halo_weapon {
    s_halo_object base;
    UNKNOWN(0xC2);  // 1f4
    WORD ammo_pack; // 2b6 reserve ammo
    WORD ammo_clip; // 2b8
    UNKNOWN(0x96);

    void SetAmmo(WORD pack, WORD clip);
    static void SyncAmmo(ident weaponId);

    // rest of weap is composed on other tags, with these tags
    // its size is 0x684
};
static_assert(sizeof(s_halo_weapon) == 0x350, "bad");

struct s_halo_vehicle {
    s_halo_object base;
    UNKNOWN(0x3b8);
    DWORD idle_timer;
    UNKNOWN(0x010);
    // rest of vehi is composed of other tags, with these tags
    // its size is 0xE00
    UNKNOWN(0xE00 - 0x5C0);
};
static_assert(sizeof(s_halo_vehicle) == 0xE00, "bad");

struct s_object_creation_disposition {
    ident map_id;
    UNKNOWN(4);
    ident player_ident;
    ident parent;
    UNKNOWN(8);
    vect3d pos;
    // rest unknown..
};

struct s_intersection_output
{
    BYTE mode; // only seen 2 (hit obj) 3 (didn't)
    UNKNOWN(0x0f);
    BYTE hit; // 0 = no hit, else hit.. i think
    UNKNOWN(7);
    vect3d hit_pos;
    UNKNOWN(0x14);
    ident hit_obj;
    UNKNOWN(0x28);
};
static_assert(sizeof(s_intersection_output) == 0x64, "bad s_intersection_test");
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

void OnObjectDestroy(ident m_objid);

}
}