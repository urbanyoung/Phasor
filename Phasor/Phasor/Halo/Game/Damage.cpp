#include "Damage.h"
#include "../../../ScriptingEvents.h"
#include "../tags.h"
#include "../../Globals.h"

namespace halo {

	// Called when an object's damage is being looked up
	bool __stdcall OnDamageLookup(s_damage_info* dmg, ident* receiver)
	{
		s_tag_entry* tag = LookupTag(dmg->tag_id);
		damage_script_options opts;
		bool allow = scripting::events::OnDamageLookup(dmg, tag->metaData, *receiver, opts);

		if (allow) {
			dmg->causer = opts.causer;
			dmg->player_causer = opts.causer_player;
			dmg->flags = opts.flags;
			dmg->tag_id = opts.tag;
			dmg->modifier1 = opts.modifier;
			*receiver = opts.receiver;

		}
		return allow;
	}

	// Called when damage is being applied to an object
	bool __stdcall OnDamageApplication(const s_damage_info* dmg, ident receiver,
		s_hit_info* hit, bool backtap)
	{
		return scripting::events::OnDamageApplication(dmg, receiver, hit, backtap);
	}

}