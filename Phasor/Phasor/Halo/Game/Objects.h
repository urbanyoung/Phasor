#pragma once

#include "../../Common/Types.h"

namespace halo { namespace objects
{
	#pragma pack(push, 1)

	struct vect3d
	{
		float x;
		float y;
		float z;
	};

	// Some structure issues were clarified thanks to the code at:
	// http://code.google.com/p/halo-devkit/source/browse/trunk/halo_sdk/Engine/objects.h
	struct G_Object // generic object header
	{
		DWORD mapId; // 0x0000
		long unk[3]; // 0x0004
		char unkBits : 2; // 0x0010
		bool ignoreGravity : 1;
		char unk1 : 4;
		bool noCollision : 1; // has no collision box, projectiles etc pass right through
		char unkBits1[3];
		unsigned long timer; // 0x0014
		char empty[0x44]; // 0x0018
		vect3d location; // 0x005c
		vect3d velocity; // 0x0068
		vect3d rotation; // 0x0074 (not sure why this is used, doesn't yaw do orientation?)
		vect3d axial; // 0x0080 (yaw, pitch, roll)
		vect3d unkVector; // 0x008C (not sure, i let server deal with it)
		char unkChunk[0x28]; // 0x0098
		unsigned long ownerPlayer; // 0x00c0 (index of owner (if has one))
		unsigned long ownerObject; // 0x00c4 (object id of owner, if projectile is player id)
		char unkChunk1[0x18]; // 0x00c8
		float health; // 0x00e0
		float shield; // 0x00e4
		char unkChunk2[0x10]; // 0x00e8
		vect3d location1; // 0x00f8 set when in a vehicle unlike other one. best not to use tho (isnt always set)
		char unkChunk3[0x10]; // 0x0104
		unsigned long veh_weaponId; // 0x0114
		unsigned long player_curWeapon; // 0x0118
		unsigned long vehicleId; // 0x011C
		BYTE bGunner; // 0x0120
		short unkShort; // 0x0121
		BYTE bFlashlight; // 0x0123
		long unkLong; // 0x0124
		float shield1; // 0x0128 (same as other shield)
		float flashlightCharge; // 0x012C (1.0 when on)
		long unkLong1; // 0x0130
		float flashlightCharge1; // 0x0134
		long unkChunk4[0x2f]; // 0x0138
	};

	struct hBiped
	{
		char unkChunk[0x10]; // 0x1F4
		long invisible; // 0x204 (0x41 inactive, 0x51 active. probably bitfield but never seen anything else referenced)
		struct aFlags // these are action flags, basically client button presses
		{  //these don't actually control whether or not an event occurs
			bool crouching : 1; // 0x0208 (a few of these bit flags are thanks to halo devkit)
			bool jumping : 1; // 2
			char unk : 2; // 3
			bool flashlight : 1; // 5
			bool unk2 : 1; // 6
			bool actionPress : 1; // 7 think this is just when they initially press the action button
			bool melee : 1; // 8
			char unk3 : 2; // 9
			bool reload : 1; // 11
			bool primaryWeaponFire : 1; // 12 right mouse
			bool secondaryWeaponFire : 1; // 13 left mouse
			bool secondaryWeaponFire1 : 1; // 14
			bool actionHold : 1; // 15 holding action button
			char unk4 : 1; // 16
		} actionFlags;
		char unkChunk1[0x26]; // 0x020A
		unsigned long cameraX; // 0x0230
		unsigned long cameraY; // 0x0234
		unsigned long cameraZ; // 0x0238
		char unkChunk2[0x6c]; // 0x23C
		BYTE bodyState; // 0x2A7 (2 = standing, 3 = crouching, 0xff = invalid, like in vehicle)
		char unkChunk3[0x50]; // 0x2A8
		unsigned long primaryWeaponId; // 0x2F8
		unsigned long secondaryWeaponId; // 0x2FC
		unsigned long tertiaryWeaponId; // 0x300
		unsigned long quartaryWeaponId; // 0x304
		char unkChunk4[0x18]; // 0x308
		BYTE zoomLevel; // 0x320 (0xff - no zoom, 0 - 1 zoom, 1 - 2 zoom etc)
		BYTE zoomLevel1; // 0x321
		char unkChunk5[0x1AA]; // 0x322
		BYTE isAirbourne; // 0x4CC (only when not in vehicle)
	};


	struct HObject // structure for objects
	{
		G_Object base;
		union
		{
			hBiped biped;
		};
	};

	// Structure of tag index table
	struct hTagIndexTableHeader
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
	struct hTagHeader
	{
		DWORD tagType; // ie weap
		DWORD unk[2]; // I don't know
		DWORD id; // unique id for map
		char* tagName; // name of tag
		LPBYTE metaData; // data for this tag
		DWORD unk1[2]; // I don't know
	};

	// Stripped down hTagHeader
	struct objInfo
	{
		DWORD tagType;
		char* tagName;
		DWORD empty;
		DWORD id;
	};

	#pragma pack(pop)

	// --------------------------------------------------------------------\
	// Events
	
	// Called when an object is being checked to see if it should respawn
	int __stdcall ObjectRespawnCheck(DWORD m_objId, LPBYTE m_obj);

	// This is called when weapons/equipment are going to be destroyed.
	// todo: check ticks should be signed
	bool __stdcall EquipmentDestroyCheck(int checkTicks, DWORD m_objId, LPBYTE m_obj);
}}