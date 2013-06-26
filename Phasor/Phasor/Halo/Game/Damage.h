#include "../Halo.h"

namespace halo {

	struct s_object_info;
	struct s_tag_entry;

	#pragma pack(push, 1)
	
	struct s_damage_info {
		ident tag_id;
		unsigned long flags;
		ident player_causer;
		ident causer; // obj of causer
		UNKNOWN(0x30);
		float modifier; // 1.0 = max dmg, < 0 decreases dmg.
		float modifier1; // 1.0 default > 1.0 increases dmg.
		UNKNOWN(8);
	};
	static_assert(sizeof(s_damage_info) == 0x50, "bad");

	
	struct s_hit_info {
		char desc[0x20];
		UNKNOWN(0x28); 	// doesn't seem to be that useful, mostly 0s with a few 1.0 floats.	
	};
	static_assert(sizeof(s_hit_info) == 0x48, "bad");

	#pragma pack(pop)
	namespace damage_flags {
		static const int kNone =		0x00;
		static const int kRespawn =		0x02;
		static const int kInstantKill = 0x04;
	}

	struct damage_script_options
	{
		halo::ident receiver;
		halo::ident causer; // obj id
		halo::ident causer_player; // player id
		halo::ident tag;
		int flags;
		float modifier;
	};

	// Called when an object's damage is being looked up
	bool __stdcall OnDamageLookup(s_damage_info* dmg, ident* receiver);

	// Called when damage is being applied to an object
	bool __stdcall OnDamageApplication(const s_damage_info* dmg, ident receiver,
		s_hit_info* hit, bool backtap);
}