#include "Damage.h"
#include "../../../ScriptingEvents.h"

namespace halo {

	// Called when an object's damage is being looked up
	bool __stdcall OnDamageLookup(s_damage_info* dmg, ident* receiver)
	{
		damage_script_options opts;
		bool allow = scripting::events::OnDamageLookup(dmg, *receiver, opts);

		if (allow) {
			dmg->causer = opts.causer;
			dmg->player_causer = opts.causer_player;
			dmg->flags = opts.flags;
			dmg->tag_id = opts.tag;
			dmg->modifier1 = opts.modifier;
			*receiver = opts.receiver;

		}
		return allow;
		//*p_receiver = causingObj;
		//*p_causer = receivingObj;
		//* 
		/*bullet: 4e3e58
		explosion: 524c20
		melee: 58545d
		server_kill (when leaving): 523fda
		fall damage: 5736be

		if 0x40 < 0 it does less damage, == 1.0 then max?

		0x44 is a damage multiplier
		input is 0x50 long
		*/
	}

}