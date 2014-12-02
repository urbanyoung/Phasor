#include "Damage.h"
#include "../../../Scripts/script-events.h"
#include "../tags.h"
#include "../../Globals.h"
#include "Objects.h"
#include "Game.h"
#include "../Server/Server.h"
#include "../../../Common/MyString.h"

namespace halo {

	damage_script_options::damage_script_options(const halo::s_damage_info* dmg,
		halo::ident receiver)
		: receiver(receiver), causer(dmg->causer), causer_player(dmg->player_causer),
		tag(dmg->tag_id), flags(dmg->flags), modifier(dmg->modifier1)
	{
	}

	void damage_script_options::copyInto(s_damage_info& dmg, ident& receiver) const
	{
		dmg.causer = causer;
		dmg.player_causer = causer_player;
		dmg.flags = flags;
		dmg.tag_id = tag;
		dmg.modifier1 = modifier;
		receiver = this->receiver;
	}

	// Called when an object's damage is being looked up
	bool __stdcall OnDamageLookup(s_damage_info* dmg, ident* receiver)
	{
		s_tag_entry* tag = LookupTag(dmg->tag_id);
		damage_script_options opts(dmg, *receiver);
		bool allow = scripting::events::OnDamageLookup(dmg, tag->metaData, *receiver, opts);

		if (allow) {
			opts.copyInto(*dmg, *receiver);
		}
		return allow;
	}

	// Called when damage is being applied to an object
	bool __stdcall OnDamageApplication(const s_damage_info* dmg, ident receiver,
		s_hit_info* hit, bool backtap)
	{
		return scripting::events::OnDamageApplication(dmg, receiver, hit, backtap);
	}

	void ApplyDamage(halo::ident receiver, halo::ident causer, 
		const s_tag_entry& dmg_tag, float mult, int flags)
	{
		//if (!receiver.valid() || objects::GetObjectAddress(receiver) == NULL || (causer.valid() && objects::GetObjectAddress(causer) == NULL))
		//	return false;

		s_damage_info dmg;
		memset(dmg._unused_4, 0, sizeof(dmg._unused_4));
		memset(dmg._unused_5, 0, sizeof(dmg._unused_5));
		*(WORD*)dmg._unused_4 = 0xFFFF;
		*(WORD*)(dmg._unused_4 + 8) = 0xFFFF;
		*(WORD*)(dmg._unused_5 + 4) = 0xFFFF;

		dmg.tag_id = dmg_tag.id;
		dmg.causer = causer;
		dmg.flags = flags;
		dmg.modifier = 1;
		dmg.modifier1 = mult;

		s_player* player_causer = game::getPlayerFromObjectId(causer);
		dmg.player_causer = player_causer ? player_causer->getPlayerIdent() : ident();

		//0018DBF8  5B 07 D1 E8 84 00 00 00 FF FF FF FF FF FF FF FF  [Ñè„...ÿÿÿÿÿÿÿÿ
		//0018DC08  FF FF 00 00 00 00 00 00 FF FF 00 00 00 00 00 00  ÿÿ......ÿÿ......
		//0018DC18  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
		//0018DC28  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
		//0018DC38  00 00 80 3F 00 00 80 3F 00 00 00 00 FF FF 00 00  ..€?..€?....ÿÿ..

		__asm {
			pushad

			push 0
			push -1
			push -1
			push -1
			push receiver
			lea edi, dmg
			push edi
			call CC_DAMAGELOOKUP
			add esp, 0x18

			popad
		}
	}

	struct DamageModifier {
		s_tag_entry* tag;
		s_tag_entry orig_tag;
		s_damage_amount orig_dmg;
		s_damage_tag* data;

		explicit DamageModifier(s_tag_entry* tag)
			: tag(tag), orig_tag(*tag)
		{
			data = (s_damage_tag*)tag->metaData;
			orig_dmg = data->amount;
		}

		~DamageModifier() {
			*tag = orig_tag;
			data->amount = orig_dmg;
		}
	};

	// Temporarily modifies globals\vehicle_hit_environment and uses it to apply damage.
	bool ApplyDamage(halo::ident receiver, halo::ident causer, float dmg, int flags)
	{
		s_tag_entry* dmg_tag = LookupTag(s_tag_type("jpt!"), "globals\\vehicle_hit_environment");
		if (!dmg_tag) {
			server::s_server_info* info = server::GetServerStruct();
			*g_PhasorLog << "Map " << info->map_name << " doesn't have globals\\vehicle_hit_environment" << endl;
			return false;
		}

		// Temporarily modify the tag
		static const char* phasor_dmg = "phasor_damage";
		DamageModifier dmgMod(dmg_tag);
		dmg_tag->tagName = phasor_dmg;

		s_damage_tag* data = (s_damage_tag*)dmg_tag->metaData;
		data->amount = s_damage_amount(1,1,1);//dmg,dmg,dmg);

		ApplyDamage(receiver, causer, *dmg_tag, dmg, flags);
		return true;
	}

}