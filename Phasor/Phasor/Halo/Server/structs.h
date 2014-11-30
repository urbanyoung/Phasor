/********************************************************************************
	HaloESP v1 -- Halo Custom Edition ESP
	Copyright (C) 2008 Del

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
*********************************************************************************
	File:	structs.h
	Original Project: HaloESP v1
	Author:  Steve(Del)
	Date:	September 23, 2008
	Game:	Halo Custom Edition
	Version: 1.08
	Modified by: Abyll, Jan 2009, for SuperApp(v0.1 and above),
		and enhanced for further functionality.
	
	Tools Used: Memory Hacking Software -- L.Spiro
				Microsoft Visual C++ 2008 Express Edition
				OllyDbg

	Credits: Evobyte -- Help with ESP Method & Concept
			 DrUnKeN ChEeTaH -- D3D
			 Strife -- D3D
			 Fatboy88 -- D3D
			 E3pO -- Testing

	Information: Website -- www.deltronzero.com
				 E-mail -- deltronzero@hotmail.com
*********************************************************************************/
#ifndef STRUCTS_H
#define STRUCTS_H
#include <windows.h>
#include <time.h>
//#include <d3dx9.h>
#pragma pack(1)
#include <d3d9.h>

//#include "player_info.h"
//-------------------------------------------

struct vect3
{
	float x;
	float y;
	float z;
};

struct vect2
{
	float x;
	float y;
};

//#define vect3 D3DXVECTOR3
//#define vect2 D3DXVECTOR2

typedef char playerindex;
typedef char toggle;

/* This looks like part of an array of some sort, esp since there is a (an assumed) pointer to it.
00 00 00 00 00 00 00 00 00 00 00 00 D0 07 00 00
00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00
D0 07 00 00 00 00 00 00 98 B7 A6 02 FF FF FF FF*/
struct Unknown0
{
	BYTE Unknown[48];
};


struct HaloVersion
{
	unsigned char ucVer[32];
};
//extern HaloVersion *Trial, *Full;
//-------------------------------------------
struct ban_entry
{
	wchar_t player_name[12];
	WORD Unknown0;				// It's a zero. Possibly for NULL string termination
	char CDhash[32];			// yeah.. Why the hell is it characters? it could be a hex value!!
	WORD Unknown1;				// It's another zero. Possibly for NULL string termination
	short BanCount;
	BYTE IsPermanent;
	unsigned long BanEndDate;	// Time that the ban ENDS (if the ban wasnt permanent and the endtime would be 0)
	// OMG, it's not a squared size memory block! Wierd!
};
struct ban_header
{
	DWORD unknown1;
	long listsize;
	ban_entry* banlist; // pointer to the array.
	DWORD unknown2;
	// some other stuff...
	// and after this, are directories for the serverlog and the banlist file.
};// @ 0x005C5204 in haloceded

//char* timetodate(unsigned long ban_date)
//{
//	time_t thetime = (time_t) ban_date;
//	static char str[21];
//	strftime(str, 20, "%Y-%m-%d %X", gmtime(&thetime));
//	return str;
//}

//time_t datetotime(char* str)
//{
//	int year, month, day, hour, minute, second;
//	tm time;
//	sscanf_s(str, "%d-%d-%d %d:%d:%d", year,month,day, hour,minute,second);
//	//I didn't finish, since I don't know how to work with gmtime yet. (Just didn't get around to it..)

//}

// Found in SV_PlayersList, Machine, and Player structs.
struct pinfo
{
	playerindex MachineIndex;	// Index to the Machine List (which has their CURRENT cdhash and IP. (it also has the LAST player's name))
	char Unknown2;		//apparently nothing. But, if these 4 chars are FF's, then the player isn't on.
	char Team;
	playerindex PlayerIndex;	// Index to their StaticPlayer
};

struct ident
{
	short index;
	short ID; // which is a Salt of index
};

bool operator==(const ident &ident1, const ident &ident2);

struct AObject;
struct s_object_data;

//-------------------------------------------
struct Object_Table_Array
{
	unsigned short ObjectID;			// Matches up to Object ID in static player table ( for players )
	unsigned short Unknown0;
	unsigned short Unknown1;
	unsigned short Size;				// Structure size
	AObject* Offset;						// Pointer to the object data structure
};
//extern Object_Table_Array *ObjectTableArray;
//-------------------------------------------
//------------------------------------------- 

struct Object_Table_Header
{
	unsigned char TName[32];			// 'object'
	unsigned short MaxObjects;			// Maximum number of objects - 0x800(2048 objects)
	unsigned short Size;				// Size of each object array - 0x0C(12 bytes)
	unsigned long Unknown0;				// always 1?
	unsigned char Data[4];				// '@t@d' - translates to 'data'?
	unsigned short Max;					// Max number of objects the game has reached (slots maybe?)
	unsigned short Num;					// Number of objects in the current game
	unsigned short NextObjectIndex;		// Index number of the next object to spawn
	unsigned short NextObjectID;		// ID number of the next object to spawn
	Object_Table_Array* Objects;		// Pointer to the first object in the table
	//Object_Table_Array Objects[2048];	// Hey, they're right here though!
};
//extern Object_Table_Header *ObjectTableHeader;
//-------------------------------------------
struct bone
{
	float unknown[10];
	vect3 World;
};

class AMasterchief
{
public:
	WORD	BipdMetaIndex;		// 0x0000	[Biped]characters\cyborg_mp\cyborg_mp
	WORD	BipdMetaID;			// 0x0002	[Biped]characters\cyborg_mp\cyborg_mp
	BYTE	Zeros_00[4];		// 0x0004
	BYTE	BitFlags_00[4];		// 0x0008
	DWORD	Timer_00;			// 0x000C
	BYTE	BitFlags_01[4];		// 0x0010
	DWORD	Timer_01;			// 0x0014
	BYTE	Zeros_01[68];		// 0x0018
	vect3	World;				// 0x005C
	vect3	Velocity;			// 0x0068
	vect3	LowerRot;			// 0x0074
	vect3	Scale;				// 0x0080
	BYTE	Zeros_02[12];		// 0x008C
	DWORD	LocationID;			// 0x0098
	DWORD	Pointer_00;			// 0x009C
	vect3	Unknown;			// 0x00A0
	BYTE	Zeros_03[20];		// 0x00AC
	short	PlayerIndex;		// 0x00C0
	short	PlayerID;			// 0x00C2
	DWORD	Unknown00;			// 0x00C4
	BYTE	Zeros_04[4];		// 0x00C8
	WORD	AntrMetaIndex;		// 0x00CC	[Animation Trigger]characters\cyborg\cyborg
	WORD	AntrMetaID;			// 0x00CE	[Animation Trigger]characters\cyborg\cyborg
	BYTE	BitFlags_02[8];		// 0x00D0
	BYTE	Unknown01[8];		// 0x00D8
	float	Health;				// 0x00E0
	float	Shield_00;			// 0x00E4
	DWORD	Zeros_05;			// 0x00E8
	float	Unknown02;			// 0x00EC
	DWORD	Unknown03;			// 0x00F0
	float	fUnknown04;			// 0x00F4
	float	fUnknown05;			// 0x00F8
	BYTE	Unknown06[24];		// 0x00FC
	short	VehicleWeaponIndex;	// 0x0114
	short	VehicleWeaponID;	// 0x0116
	short	WeaponIndex;		// 0x0118
	short	WeaponID;			// 0x011A
	short	VehicleIndex;		// 0x011C	Ex: Turret on Warthog
	short	VehicleID;			// 0x011E
	short	SeatType;			// 0x0120
	BYTE	BitFlags_03[2];		// 0x0122
	DWORD	Zeros_06;			// 0x0124
	float	Shield_01;			// 0x0128
	float	Flashlight_00;		// 0x012C
	float	Zeros_07;			// 0x0130
	float	Flashlight_01;		// 0x0134
	BYTE	Zeros_08[20];		// 0x0138
	DWORD	Unknown07;			// 0x014C	
	BYTE	Zeros_09[28];		// 0x0150
	BYTE	Unknown08[8];		// 0x016C
	BYTE	Unknown10[144];		// 0x0174
	DWORD	IsInvisible;		// 0x0204	normal = 0x41 invis = 0x51 (bitfield?)
	BYTE	IsCrouching;		// 0x0208	crouch = 1, jump = 2
	BYTE	Unknown11[3];		// 0x0209
	BYTE	Unknown09[276];		// 0x020C
	BYTE	Zoom00;				// 0x0320
	BYTE	Zoom01;				// 0x0321
	BYTE	Unknown12[610];		// 0x0322
	bone	LeftThigh;			// 0x0584
	bone	RightThigh;			// 0x05B8
	bone	Pelvis;				// 0x05EC
	bone	LeftCalf;			// 0x0620
	bone	RightCalf;			// 0x0654
	bone	Spine;				// 0x0688
	bone	LeftClavicle;		// 0x06BC
	bone	LeftFoot;			// 0x06F0
	bone	Neck;				// 0x0724
	bone	RightClavicle;		// 0x0758
	bone	RightFoot;			// 0x078C
	bone	Head;				// 0x07C0
	bone	LeftUpperArm;		// 0x07F4
	bone	RightUpperArm;		// 0x0828
	bone	LeftLowerArm;		// 0x085C
	bone	RightLowerArm;		// 0x0890
	bone	LeftHand;			// 0x08C4
	bone	RightHand;			// 0x08F8
	BYTE	Unknown20[1216];	// 0x092C
}; // Size - 3564(0x0DEC) bytes
//extern AMasterchief *Masterchief;
//extern AMasterchief *LocalMC;

struct AObject
{
	ident	ModelTag;		// 0x0000
	long	Zero;			// 0x0004
	char	Flags[4];		// 0x0008
	long	Timer;			// 0x000C
	char	Flags2[4];		// 0x0010
	long	Timer2;			// 0x0014
	long	Zero2[17];		// 0x0018
	vect3	World;			// 0x005C
	vect3	Velocity;		// 0x0068
	vect3	LowerRot;		// 0x0074
	vect3	Scale;			// 0x0080
	vect3	UnknownVect1;	// 0x008C
	long	LocationID;		// 0x0098
	long	Unknown1;		// 0x009C
	vect3	UnknownVect2;	// 0x00A0
	float	Unknown2[2];	// 0x00AC		
	long	Unknown3[3];	// 0x00B4
	ident	Player;			// 0x00C0
  ident	Owner;      // 0x00C4
	//long	Unknown4[2];	// 0x00C4
  long	Unknown4[1];
	ident	AntrMeta;		// 0x00CC
	long	Unknown5[4];	// 0x00D0
	float	Health;			// 0x00E0
	float	Shield1;		// 0x00E4
	long	Unknown6[11];	// 0x00E8
	ident	VehicleWeapon;	// 0x0114
	ident	Weapon;			// 0x0118
	ident	Vehicle;		// 0x011C
	short	SeatType;		// 0x0120
	short	Unknown7;		// 0x0122
	long	Unknown8;		// 0x0124
	float	Shield2;		// 0x0128
	float	Flashlight1;	// 0x012C
	float	Unknown9;		// 0x0130
	float	Flashlight2;	// 0x0134
	long	Unknown10[5];	// 0x0138
	ident	UnknownIdent1;	// 0x014C
	ident	UnknownIdent2;	// 0x0150
	long	Zero3[6];		// 0x0154
	ident	UnknownIdent3;	// 0x016C
	ident	UnknownIdent4;	// 0x0170
	D3DMATRIX UnknownMatrix;	// 0x0174
	D3DMATRIX UnknownMatrix1;	// 0x01B4
};

struct ABiped
{
	ident	ModelTag;		// 0x0000
	long	Zero;			// 0x0004
	char	Flags[4];		// 0x0008
	long	Timer;			// 0x000C
	char	Flags2[4];		// 0x0010
	long	Timer2;			// 0x0014
	long	Zero2[17];		// 0x0018
	vect3	World;			// 0x005C
	vect3	Velocity;		// 0x0068
	vect3	LowerRot;		// 0x0074
	vect3	Scale;			// 0x0080
	vect3	UnknownVect1;	// 0x008C
	long	LocationID;		// 0x0098
	long	UnknownO1;		// 0x009C
	vect3	UnknownVect2;	// 0x00A0
	float	UnknownO2[2];	// 0x00AC		
	long	UnknownO3[3];	// 0x00B4
	ident	Player;			// 0x00C0
	long	UnknownO4[2];	// 0x00C4
	ident	AntrMeta;		// 0x00CC
	long	UnknownO5[4];	// 0x00D0
	float	Health;			// 0x00E0
	float	Shield1;		// 0x00E4
	long	UnknownO6[11];	// 0x00E8
	ident	VehicleWeapon;	// 0x0114
	ident	Weapon;			// 0x0118
	ident	Vehicle;		// 0x011C
	short	SeatType;		// 0x0120
	short	UnknownO7;		// 0x0122
	long	UnknownO8;		// 0x0124
	float	Shield2;		// 0x0128
	float	Flashlight1;	// 0x012C
	float	Unknown9;		// 0x0130
	float	Flashlight2;	// 0x0134
	long	UnknownO10[5];	// 0x0138
	ident	UnknownIdent1;	// 0x014C
	ident	UnknownIdent2;	// 0x0150
	long	Zero3[6];		// 0x0154
	ident	UnknownIdent3;	// 0x016C
	ident	UnknownIdent4;	// 0x0170
	D3DMATRIX UnknownMatrix;	// 0x0174
	D3DMATRIX UnknownMatrix1;	// 0x01B4
//** END OBJECT part
	long	Unknown[4];			// 0x01F4
	short	IsInvisible;		// 0x0204	normal = 0x41 invis = 0x51 (bitfield?)
	char	Flashlight;			// 0x0206
	char	Frozen;				// 0x0207
	char	IsCrouching :	1;	// 0x0208
	char	IsJumping	:	1;
	char	UnknownBit	:	2;
	char	Flashligh	:	1;
	char	UnknownBit2	:	3;
	char	ActionBits;			// 0x0209
	char	Unknown1[2];		// 0x020A
	long	UnknownCounter1;	// 0x020C
	long	UnknownLongs1[5];	// 0x0210
	vect3	RightVect;			// 0x0224
	vect3	UpVect;				// 0x0230
	vect3	LookVect;			// 0x023C
	vect3	ZeroVect;			// 0x0248
	vect3	RealLookVect;		// 0x0254
	vect3	UnknownVect3;		// 0x0260
	char	Unknown2[132];		// 0x026C
	ident	PrimaryWeapon;		// 0x0318
	ident	SecondaryWeapon;	// 0x031C
	ident	ThirdWeapon;
	ident	FourthWeapon;
	long	UnknownLongs2[6];
	char	Zoom;				// 0x0320
	char	Zoom1;				// 0x0321
	char	Unknown3[558];		// 0x0322
	bone	LeftThigh;			// 0x0584
	bone	RightThigh;			// 0x05B8
	bone	Pelvis;				// 0x05EC
	bone	LeftCalf;			// 0x0620
	bone	RightCalf;			// 0x0654
	bone	Spine;				// 0x0688
	bone	LeftClavicle;		// 0x06BC
	bone	LeftFoot;			// 0x06F0
	bone	Neck;				// 0x0724
	bone	RightClavicle;		// 0x0758
	bone	RightFoot;			// 0x078C
	bone	Head;				// 0x07C0
	bone	LeftUpperArm;		// 0x07F4
	bone	RightUpperArm;		// 0x0828
	bone	LeftLowerArm;		// 0x085C
	bone	RightLowerArm;		// 0x0890
	bone	LeftHand;			// 0x08C4
	bone	RightHand;			// 0x08F8
	char	Unknown4[1216];		// 0x092C
}; // Size - 3564(0x0DEC) bytes

////////////////////
// Generic Weapon //
////////////////////
struct AWeapon
{
	AObject			Object;
	char			Unknown[24];
	ident			UnknownIdent;
	unsigned long	NetworkTime;
	char			Unknown1[36];
	unsigned long	UnknownFlags	:	32;
	unsigned long	UnknownBit		:	1;
	unsigned long	Fire			:	1;
	unsigned long	Grenade			:	1;
	unsigned long	Reload			:	1;
	unsigned long	Melee			:	1;
	unsigned long	UnknownBit1		:	1;
	unsigned long	Zoom			:	1;
	unsigned long	UnknownBit2		:	1;
	unsigned long	UnusedBits		:	24;
	float			Unknown2;
	bool			IsFiring;
	char			Unknown3;
	unsigned short	WeaponReadyWaitTime;
	char			Unknown4[36];
	unsigned long	SomeCounter;
	unsigned long	IsNotFiring;
	unsigned long	Unknown5[2];
	float			Unknown6;
	unsigned long	Unknown7;
	float			Unknown8[2];
	ident			UnknownIdent1;
	unsigned long	AutoReloadCounter;
	unsigned char	Unknown9[28];
	unsigned short	ReloadFlags; // 0=NotReloading,1=Reloading, 2=???, 3=???
	unsigned short	ReloadCountdown;
	unsigned short	Unknown10;
	unsigned short	BulletCountInRemainingClips;
	unsigned short	BulletCountInCurrentClip;
	char			Unknown11[18];
	ident			UnknownIdent2;
	unsigned long	LastBulletFiredTime;
	char			Unknown12[16];
	vect3			Unknown13[2];
	char			Unknown14[12];
	unsigned long	BulletCountInRemainingClips1;
	char			Unknown15[52];
}; // Size - 1644(0x066C)

//-------------------------------------------
struct Static_Player
{
	short	PlayerID;
	short	IsLocal;			// 0=Local(no bits set), -1=Other Client(All bits set)
	wchar_t Name[12];			// Unicode
	ident	UnknownIdent;
	long	Team;				// 0=Red, 1=Blue
	ident	SwapObject;
	short	SwapType;
	short	SwapSeat;			// Warthog-Driver=0, Passenger=1, Gunner=2, Weapon=-1
	long	RespawnTimer;		// Counts down when dead, Alive=0
	long	Unknown;
	ident	CurrentBiped;
	ident	PreviousBiped;
	short	ClusterIndex;
	char	Swap : 1;
	char	UnknownBits4 :7;
	char	UnknownByte;
	ident	UnknownIdent1;
	long	LastBulletShotTime;	// since game start(0)
	wchar_t Name1[12];
	ident	UnknownIdent2;
	//pinfo PlayerInfo;
	playerindex	MachineIndex;		// Index to the Machine List (which has their CURRENT cdhash and IP. (it also has the LAST player's name))
	char	Unknown0;			//something. But, if these 4 chars are FF's, then the player isn't on.
	char	iTeam;
	playerindex	PlayerIndex;		// Index to their StaticPlayer
	long	Unknown1;
	float	VelocityMultiplier;
	ident	UnknownIdent3[4];
	long	Unknown2;
	long	LastDeathTime;		// since game start(0)
	char	Unknown3[18];
	short	KillsCount;
	char	Unknown4[6];
	short	AssistsCount;
	char	Unknown5[8];
	short	BetrayedCount;
	short	DeathsCount;
	short	SuicideCount;
	char	Unknown6[18];
	short	FlagStealCount;
	short	FlagReturnCount;
	short	FlagCaptureCount;
	char	Unknown7[6];
	ident	UnknownIdent4;
	char	Unknown8[8];
	short	Ping;
	char	Unknown9[14];
	ident	UnknownIdent5;
	long	Unknown10;
	long	SomeTime;
	vect3	World;
	ident	UnknownIdent6;
	char	Unknown11[20];
	char	Melee		:	1;
	char	Action		:	1;
	char	UnknownBit	:   1;
	char	Flashlight	:	1;
	char	UnknownBit1	:	4;
	char	UnknownBit2	:	5;
	char	Reload		:	1;
	char	UnknownBit3	:	2;
	char	Unknown12[26];
	vect2	Rotation;		// Yaw, Pitch (again, in radians.
	float	ForwardVelocityMultiplier;
	float	HorizontalVelocityMultiplier;
	float	RateOfFireVelocityMultiplier;
	short	HeldWeaponIndex;
	short	GrenadeIndex;
	char	Unknown13[4];
	vect3	LookVect;
	char	Unknown14[16];
	vect3	WorldDelayed;	// Oddly enough... it matches the world vect, but seems to lag behind (Possibly what the client reports is _its_ world coord?)
	char	Unknown15[132];
};
//extern Static_Player *StaticPlayer;
//extern Static_Player *LocalPlayer;
//-------------------------------------------
struct Static_Player_Header
{
	char TName[32];		// 'players'
	short MaxSlots;		// Max number of slots/players possible
	short SlotSize;		// Size of each Static_Player struct
	long Unknown;			// always 1?
	char Data[4];			// '@t@d' - translated as 'data'?
	short IsInMainMenu;	// 0 = in game 1 = in main menu / not in game
	short SlotsTaken;		// or # of players
	short NextPlayerIndex;	// Index # of the next player to join
	short NextPlayerID;	// ID # of the next player to join
	Static_Player* Players;			// Pointer to the first static player, always 0x4029CEC8
	//Static_Player Players[16];		// It just so happens theyre right here though.
};
//extern Static_Player_Header *StaticPlayerHeader;
//-------------------------------------------
/// (some other player Header struct, apparently for Server side listing.
//-------------------------------------------

struct MachineInfo
{
  void*	UnknownPtr; // Following this, i found a pointer next to other useless stuff. Then, another pointer, then i found some stuff that looked like it /MIGHT/ very well be strings related to a hash. *shrug*
  long	Unknown0[2];
  short	Unknown8;
  short	Unknown9;
  long	Unknown10[3];
  long	Unknown11;	// most of the time 1, but sometimes changes to 2 for a moment.
  long	Unknown12;
  // 16 bit bitfield for action keys:
  char	Crouch	: 1;
  char	Jump	: 1;
  char	Flashlight : 1;
  char	Unknownbit0 :1;
  char	Unknownbit1 :1;
  char	Unknownbit2 :1;
  char	Unknownbit3 :1;
  char	Unknownbit4 :1;
  char	Reload	: 1;
  char	Fire	: 1;
  char	Swap	: 1;
  char	Grenade	: 1;
  char	Unknownbit5 :1;
  char	Unknownbit6 :1;
  char	Unknownbit7 :1;
  char	Unknownbit8 :1;

  short	Unknown13;
  float	Yaw;			// player's rotation - in radians, from 0 to 2*pi, (AKA heading)
  float	Pitch;			// Player's pitch - in radians, from -pi/2 to +pi/2, down to up. 
  float	Roll;			// roll - unless walk-on-walls is enabled, this will always be 0.
  BYTE	Unknown1[8];
  float	ForwardVelocityMultiplier;
  float	HorizontalVelocityMultiplier;
  float	ROFVelocityMultiplier;
  short	WeaponIndex;
  short	GrenadeIndex;
  short	UnknownIndex;		// The index is -1 if no choices are even available.
  short	Unknown2;
  short	Unknown3;			// 1
  char	EncryptedString[8];	// I just assume it's a string - they're only character values, and it's NULL terminated I think. But, those letters don't make any sense.
  short	Unknown4;
  long	PlayCount;			// it went up every time I rejoined. I don't know if that's for _That_ player, or for just that slot.
  wchar_t	LastPlayersName[12];	// Odd.. this isnt the name of the player who's on, but i thinkn it's the Previous player's name.
  long	Unknown6;			// these two were -1.
  long	Unknown7;			// but sometimes become 0.
  char	IP[32];
  char	CDhash[32];			// a solid block array, so it's not necessarily a c_str i think, but there's still usually just 0's afterwards anyways.
  BYTE	UnknownZeros[44];	// zeros..
}; // Size: 0xFE

struct Alter_Player
{
	wchar_t PlayerName[12];
	short ColorIndex;
	short Unknown1;		// FFFF
	//pinfo PlayerIndexInfo;
	playerindex MachineIndex;	// Index to the Machine List (which has their CURRENT cdhash and IP. (it also has the LAST player's name))
	char Unknown2;		//something. But, if these 4 chars are FF's, then the player isn't on.
	char Team;
	playerindex PlayerIndex;	// Index to their StaticPlayer
};
//extern Alter_Player *AlterPlayer;





// @ address 006C78E0
struct GlobalServer
{
	Unknown0* Unknown0; // at least I _think_ it's a pointer since there _is_ something if i follow it.
	WORD Unknown1;
	WORD Unknown2;
	wchar_t SV_NAME[66]; // 140
	char MAP_NAME[128];
	wchar_t GAMETYPE_NAME[48];
	BYTE Unknown3[169];
	char SV_MAXPLAYERS;	// Note: there is another place that also says MaxPlayers - i think it's the ServerInfo socket buffer.
	short Unknown09;	// possibly Is_Server_Full, or Slots_Available.
	short Unknown10;	// i think LastSlotFilled
	Alter_Player PlayersList[16];
	short Unknown4;
	short Unknown5;
	short Unknown6;
	long Unknown7;
	long Unknown8;
	MachineInfo MachineList[16];
	//offset: 0x006C8B98
	// Beyond this, nothing seems apparently useful.

}; // Size: unknown





#endif /* STRUCTS_H */