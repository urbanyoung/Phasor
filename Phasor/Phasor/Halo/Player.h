#pragma once

#include "../../Common/Types.h"

namespace halo
{
	#pragma pack(push, 1)
	struct PlayerStructure
	{
		WORD playerJoinCount; // 0x0000
		WORD localClient; // 0x0002 always FF FF on a dedi in Halo is 00 00 if its your player
		wchar_t playerName[12]; //0x0004
		DWORD unk; // 0x001C only seen FF FF FF FF
		DWORD team; // 0x0020
		DWORD m_interactionObject; // 0x0024 ie Press E to enter THIS vehicle
		WORD interactionType; // 0x0028 8 for vehicle, 7 for weapon
		WORD interactionSpecifier; // 0x002A which seat of car etc
		DWORD respawnTimer; // 0x002c
		DWORD unk2; // 0x0030 only seen empty
		DWORD m_playerObjectid; // 0x0034
		DWORD m_oldObjectid; // 0x0038
		DWORD unkCounter; // 0x003C sometimes changes as you move, fuck idk
		DWORD empty; // 0x0040 always FF FF FF FF, never accessed
		DWORD bulletUnk; // 0x0044 changes when the player shoots
		wchar_t playerNameAgain[12]; // 0x0048
		WORD playerNum_NotUsed; // 0x0060 seems to be the player number.. never seen it accessed tho
		WORD empty1; // 0x0062 byte alignment
		BYTE playerNum; // 0x0064 player number used for rcon etc (ofc this is 0 based tho)
		BYTE unk_PlayerNumberHigh; // 0x0065 just a guess
		BYTE team_Again; // 0x0066
		BYTE unk3; // 0x0067  idk, probably something to do with player numbers
		DWORD unk4; // 0x0068 only seen 0, it wont always be 0 tho
		float speed; // 0x006C
		DWORD unk5[11]; // 0x0070 16 bytes of FF? then 4 0, some data, rest 0... meh idk about rest either
		WORD kills; // 0x009C
		WORD idk; // 0x009E byte alignment?
		DWORD unk6; // 0x00A0 maybe to do with scoring, not sure
		WORD assists; // 0x00A4
		WORD unk7; // 0x00A6 idk byte alignment?
		DWORD unk8; // 0x00A8
		WORD betrayals; // 0x00AC
		WORD deaths; // 0x00AE
		WORD suicides; // 0x00B0
		// cbf with the rest
		BYTE rest[0x14E]; // 0x00B2
	};
	#pragma pack(pop)

	struct Player
	{
		std::string hash;
		int memoryId;
		PlayerStructure* mem;

		// ----------------------------------------------------------------
		Player(int memoryId);
	};
}