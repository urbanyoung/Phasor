#include "damagelookup.h"
#include "api_readers.h"
#include "../Phasor/Halo/Game/Damage.h"
#include "../Phasor/Halo/Game/Game.h"

using namespace Common;
using namespace Manager;

namespace odl {

	halo::damage_script_options* opts = 0;
	//bool causer_set, receiver_set, tag_set, instakill_set, flags_set, modifier_set;
	// Track the states so that the script that first modifies a value can
	// modify it again.
	const Manager::ScriptState* flags_state, *causer_state, *receiver_state, *tag_state, *modifier_state;

	void setData(halo::damage_script_options* opts_, 
		halo::s_damage_info* dmg, const halo::ident& receiver) {
		opts = opts_;
		opts_->causer = dmg->causer;
		opts_->causer_player = dmg->player_causer;
		opts_->receiver = receiver;
		opts_->tag = dmg->tag_id;
		opts_->flags = dmg->flags;
		opts_->modifier = dmg->modifier1;

		//causer_set = receiver_set = tag_set = instakill_set =  modifier_set = flags_set = false;
		flags_state = causer_state = receiver_state = tag_state = modifier_state = NULL;
	}	

	void reset() {
		opts = 0;
	}
}

inline bool canSet(const ScriptState** state, const CallHandler& handler) {
	return odl::opts!= 0 && (*state == 0 || *state == &handler.state);
}
bool setObjectIdent(CallHandler& handler, const Object& obj, 
		halo::ident& id, const ScriptState** state) {
	if (!canSet(state, handler)) return false;
	ReadHaloObject(handler, obj, true, id);
	*state = &handler.state;
	return true;
}

bool setTagIdent(CallHandler& handler, const Object& obj, 
	halo::ident& id, const ScriptState** state) {
	if (!canSet(state, handler)) return false;
	ReadHaloTag(handler, obj, id);
	*state = &handler.state;
	return true;
}

void l_odl_causer(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	if (setObjectIdent(handler, *args[0], odl::opts->causer, &odl::causer_state)) {
		halo::s_player* player = halo::game::getPlayerFromObjectId(odl::opts->causer);
		if (player) odl::opts->causer_player = player->getPlayerIdent();
		else odl::opts->causer_player = halo::ident();
	}
}

void l_odl_receiver(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	setObjectIdent(handler, *args[0], odl::opts->receiver, &odl::receiver_state);
}

void l_odl_tag(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	setTagIdent(handler, *args[0], odl::opts->tag, &odl::tag_state);
}

void l_odl_multiplier(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	if (!canSet(&odl::modifier_state, handler)) return;

	float modifier = ReadNumber<float>(*args[0]);
	odl::opts->modifier = modifier;
	odl::modifier_state = &handler.state;
}

void l_odl_flags(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	if (!canSet(&odl::flags_state, handler)) return;
	DWORD bit_offset = ReadNumber<DWORD>(*args[0]);
	DWORD flag = 1 << bit_offset;

	// suicide bit causes instability.. ie if you set it then nade an environment object.
	if (flag == 0x80) return;
	if (args.size() == 1) {
		bool val = (odl::opts->flags & flag) == flag;
		return AddResultBool(val, results);
	} else {
		bool to_set = ReadBoolean(*args[1]);
		if (to_set) odl::opts->flags |= flag;
		else odl::opts->flags ^= flag;

		odl::flags_state = &handler.state;
	}
}
