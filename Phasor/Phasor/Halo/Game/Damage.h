#include "../Halo.h"

namespace halo {

	struct s_object_info;
	struct s_tag_entry;

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

	namespace damage_flags {
		static const int kNone =		0x00;
		static const int kIdk =			0x02;
		static const int kInstantKill = 0x04;
		static const int kSuicide =		0x40;
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

}