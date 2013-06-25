#include "damagelookup.h"
#include "api_readers.h"
#include "../Phasor/Halo/Game/Damage.h"
#include "../Phasor/Halo/Game/Game.h"

using namespace Common;
using namespace Manager;

namespace odl {

	halo::damage_script_options* opts;
	bool causer_set, receiver_set, tag_set, instakill_set, suicide_set, modifier_set;
	
	void resetData(halo::damage_script_options* opts_, 
		halo::s_damage_info* dmg, const halo::ident& receiver) {
		opts = opts_;
		opts_->causer = dmg->causer;
		opts_->causer_player = dmg->player_causer;
		opts_->receiver = receiver;
		opts_->tag = dmg->tag_id;
		opts_->flags = dmg->flags;
		opts_->modifier = 1.0f;

		causer_set = receiver_set = tag_set = instakill_set = suicide_set = modifier_set = false;
	}	
}

bool setIdent(CallHandler& handler, const Object& obj, 
		halo::ident& id, bool* bSet) {
	if (*bSet) return false;
	ReadHaloObject(handler, obj, true, id);
	*bSet = true;
	return true;
}

bool setFlag(bool value, int flag, bool* bSet) {
	if (*bSet) return false;
	if (value) odl::opts->flags |= flag;
	else odl::opts->flags ^= flag;
	*bSet = true;
	return true;
}

bool getFlag(int flag) {
	return (odl::opts->flags & flag) == 1;
}

void l_odl_causer(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	if (setIdent(handler, *args[0], odl::opts->causer, &odl::causer_set)) {
		halo::s_player* player = halo::game::getPlayerFromObjectId(odl::opts->causer);
		if (player) odl::opts->causer_player = player->getPlayerIdent();
		else odl::opts->causer_player = halo::ident();
	}
}

void l_odl_receiver(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	setIdent(handler, *args[0], odl::opts->receiver, &odl::receiver_set);
}

void l_odl_tag(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	setIdent(handler, *args[0], odl::opts->tag, &odl::tag_set);
}

void l_odl_multiplier(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	if (odl::modifier_set) return;
	float modifier = ReadNumber<float>(*args[0]);
	odl::opts->modifier = modifier;
	odl::modifier_set = true;
}

void l_odl_flags_instantkill(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	bool old = getFlag(halo::damage_flags::kInstantKill);
	if (!results.size()) return AddResultBool(old, results);
	
	if (setFlag(ReadBoolean(*args[0]), halo::damage_flags::kInstantKill, &odl::instakill_set))
		AddResultBool(old, results);
	else AddResultNil(results);
}

void l_odl_flags_suicide(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	bool old = getFlag(halo::damage_flags::kSuicide);
	if (!results.size()) return AddResultBool(old, results);

	if (setFlag(ReadBoolean(*args[0]), halo::damage_flags::kSuicide, &odl::instakill_set))
		AddResultBool(old, results);
	else AddResultNil(results);
}
