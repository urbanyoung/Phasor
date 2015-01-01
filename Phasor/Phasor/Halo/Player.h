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
		wchar_t playerName[12]; //0x0004
		UNKNOWN(4); // 0x001C only seen FF FF FF FF
		BYTE team; // 0x0020
		UNKNOWN(3); // padding?
		ident m_interactionObject; // 0x0024 ie Press E to enter THIS vehicle
		WORD interactionType; // 0x0028 8 for vehicle, 7 for weapon
		WORD interactionSpecifier; // 0x002A which seat of car etc
		DWORD respawnTimer; // 0x002c
		UNKNOWN(4); // 0x0030 only seen empty
		ident object_id; // 0x0034
		ident old_object_id; // 0x0038
		UNKNOWN(4);; // 0x003C sometimes changes as you move, fuck idk
		UNKNOWN(4); // 0x0040 always FF FF FF FF, never accessed
		UNKNOWN(4); // 0x0044 changes when the player shoots
		wchar_t playerNameAgain[12]; // 0x0048
		WORD playerNum_NotUsed; // 0x0060 seems to be the player number.. never seen it accessed tho
		WORD empty1; // 0x0062 byte alignment
		BYTE playerNum; // 0x0064 player number used for rcon etc (ofc this is 0 based tho)
		BYTE unk_PlayerNumberHigh; // 0x0065 just a guess
		BYTE team_Again; // 0x0066
        BYTE memoryId;
		UNKNOWN(4);
		float speed; // 0x006C
		UNKNOWN(0x2c); 
		WORD kills; // 0x009C
		UNKNOWN(6);
		WORD assists; // 0x00A4
		UNKNOWN(6); // A6
		WORD betrayals; // 0x00AC
		WORD deaths; // 0x00AE
		WORD suicides; // 0x00B0
        UNKNOWN(0x2a); // B2
        DWORD ping; // DC
		// cbf with the rest
		UNKNOWN(0x120); // E0
	};
	static_assert(sizeof(s_player_structure) == 0x0200, "bad");
	#pragma pack(pop)

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