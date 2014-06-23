#include "../Halo.h"
#include "../../../Common/Streams.h"

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

	struct s_damage_amount
	{
		float min_damage;
		float max_damage_min;
		float max_damage_max;

		s_damage_amount()
			: min_damage(0), max_damage_max(0), max_damage_min(0)
		{
		}

		s_damage_amount(float min_damage, float max_damage_min, float max_damage_max)
			: min_damage(min_damage), max_damage_min(max_damage_min), 
			  max_damage_max(max_damage_max)
		{
		}
	};
	static_assert(sizeof(s_damage_amount) == 0x0c, "bad");

	struct s_damage_tag
	{
		float min_range;
		float max_range;
		UNKNOWN(0x1BE);
		short damage_id;
		UNKNOWN(0x08);
		s_damage_amount amount;
		UNKNOWN(0xC4);
	};
	static_assert(sizeof(s_damage_tag) == 0x2A0, "bad");
	
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

		damage_script_options(const halo::s_damage_info* dmg, halo::ident receiver);

		void copyInto(s_damage_info& dmg, ident& receiver) const;
	};

	// Called when an object's damage is being looked up
	bool __stdcall OnDamageLookup(s_damage_info* dmg, ident* receiver);

	// Called when damage is being applied to an object
	bool __stdcall OnDamageApplication(const s_damage_info* dmg, ident receiver,
		s_hit_info* hit, bool backtap);

	// Temporarily modifies globals\vehicle_hit_environment and uses it to apply damage.
	bool ApplyDamage(halo::ident receiver, halo::ident causer, 
		float dmg, int flags);

	// Apply the specified damage tag to the receiver
	void ApplyDamage(halo::ident receiver, halo::ident causer, 
		const halo::s_tag_entry& dmg_tag, float multiplier, int flags);
}