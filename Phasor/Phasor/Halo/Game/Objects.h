#pragma once

#include "../../../Common/Types.h"
#include "../../../Common/vect3d.h"
#include "../Halo.h"

namespace halo { namespace objects
{
	#pragma pack(push, 1)

	// Some structure issues were clarified thanks to the code at:
	// http://code.google.com/p/halo-devkit/source/browse/trunk/halo_sdk/Engine/objects.h
	// 

	struct s_halo_object_header // generic object header
	{
		ident map_id; // 0x0000
		UNKNOWN(12);
		UNKNOWN_BITFIELD(2);
		bool ignoreGravity : 1;
		UNKNOWN_BITFIELD(4);
		bool noCollision : 1; // has no collision box, projectiles etc pass right through
		UNKNOWN(3);
		unsigned long timer; // 0x0014
		UNKNOWN(0x44);// empty;
		vect3d location; // 0x005c
		vect3d velocity; // 0x0068
		vect3d rotation; // 0x0074
		UNKNOWN(sizeof(vect3d)); // some vector
		UNKNOWN(sizeof(vect3d)); // some vector
		UNKNOWN(0x28);
		unsigned long ownerPlayer; // 0x00c0 (index of owner (if has one))
		ident ownerObject; // 0x00c4 (object id of owner, if projectile is player id)
		UNKNOWN(0x18);
		float health; // 0x00e0
		float shield; // 0x00e4
		UNKNOWN(0x10);
		vect3d location1; // 0x00f8 set when in a vehicle unlike other one. best not to use tho (isnt always set)
		UNKNOWN(0x10);
		unsigned long veh_weaponId; // 0x0114
		unsigned long player_curWeapon; // 0x0118
		unsigned long vehicleId; // 0x011C
		BYTE bGunner; // 0x0120
		short unkShort; // 0x0121
		BYTE bFlashlight; // 0x0123
		UNKNOWN(4);
		float shield1; // 0x0128 (same as other shield)
		float flashlightCharge; // 0x012C (1.0 when on)
		UNKNOWN(4);
		float flashlightCharge1; // 0x0134
		UNKNOWN(0xBC);
	};
	static_assert (sizeof(s_halo_object_header) == 0x1f4, "bad");

	struct s_halo_biped
	{
		s_halo_object_header base;
		UNKNOWN(0x10); // 0x1f4
		long invisible; // 0x204 (0x41 inactive, 0x51 active. probably bitfield but never seen anything else referenced)
		
		struct // 0x208
		{
			// these are action flags, basically client button presses
			// these don't actually control whether or not an event occurs
			
			bool crouching : 1; // 0
			bool jumping : 1; // 1
			UNKNOWN_BITFIELD(2);
			bool flashlight : 1; // 4
			UNKNOWN_BITFIELD(1);
			bool actionPress : 1; // 6 think this is just when they initially press the action button
			bool melee : 1; // 7
			UNKNOWN_BITFIELD(2);
			bool reload : 1; // 10
			bool primaryWeaponFire : 1; // 11 right mouse
			bool secondaryWeaponFire : 1; // 12 left mouse
			bool secondaryWeaponFire1 : 1; // 13
			bool actionHold : 1; // 14 holding action button
			UNKNOWN_BITFIELD(1);
		} actionFlags;
		UNKNOWN(0x26); // 0x020A
		_vect3d cameraView; // 230
		UNKNOWN(0x68); // 0x23c
		BYTE bodyState; // 0x2A4 (2 = standing, 3 = crouching, 0xff = invalid, like in vehicle)
		UNKNOWN(0x53); // 0x2A5
		unsigned long primaryWeaponId; // 0x2F8
		unsigned long secondaryWeaponId; // 0x2FC
		unsigned long tertiaryWeaponId; // 0x300
		unsigned long quartaryWeaponId; // 0x304
		UNKNOWN(0x18); // 0x308
		BYTE zoomLevel; // 0x320 (0xff - no zoom, 0 - 1 zoom, 1 - 2 zoom etc)
		BYTE zoomLevel1; // 0x321
		UNKNOWN(0x1AA); // 0x322
		BYTE isAirbourne; // 0x4CC (from jumping/falling, not when flying in vehicle)
		UNKNOWN(0x33); // 0x4cd - 0x500

		// The rest of bipd is composed of other tags, such as
		// mod2\characters\cyborg\cyborg
		// with these tags its size is 0xDEC
	};
	static_assert(sizeof(s_halo_biped) == 0x500, "bad");

	// entry in the object table
	struct s_halo_object
	{
		WORD id;
		char flags; // 0x44 by default, dunno what they're for.
		char type;
		UNKNOWN(2);
		WORD size;
		union
		{
			void*					data;
			s_halo_object_header*	base;
			s_halo_biped*			biped;
		};
	};
	static_assert(sizeof(s_halo_object) == 0x0c, "s_halo_object_entry incorrect");

	// Structure of tag index table
	struct s_tag_index_table_header
	{
		DWORD next_ptr;
		DWORD starting_index; // ??
		DWORD unk;
		DWORD entityCount;
		DWORD unk1;
		DWORD readOffset;
		BYTE unk2[8];
		DWORD readSize;
		DWORD unk3;
	};

	// Structure of the tag header (entry in tag table)
	struct s_tag_header
	{
		DWORD tagType; // ie weap
		DWORD unk[2]; // I don't know
		DWORD id; // unique id for map
		char* tagName; // name of tag
		LPBYTE metaData; // data for this tag
		DWORD unk1[2]; // I don't know
	};

	// Stripped down hTagHeader
	struct s_object_info
	{
		DWORD tagType;
		char* tagName;
		DWORD empty;
		DWORD id;
	};

	#pragma pack(pop)

	s_halo_object* GetObjectAddress(ident objectId);

	// --------------------------------------------------------------------\
	// Events
	
	// Called when an object is being checked to see if it should respawn
	int __stdcall ObjectRespawnCheck(DWORD m_objId, LPBYTE m_obj);

	// This is called when weapons/equipment are going to be destroyed.
	// todo: check ticks should be signed
	bool __stdcall EquipmentDestroyCheck(int checkTicks, DWORD m_objId, LPBYTE m_obj);
}}