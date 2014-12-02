#include "../phasor-lua.hpp"
#include "../../Phasor/Halo/Game/Damage.h"
#include "../../Phasor/Halo/Game/Game.h"

halo::damage_script_options* opts = nullptr;

enum class e_types {
    kFlags,
    kCauser,
    kReceiver,
    kTag,
    kModifier
};

struct StateTracker {
    const lua_State *flags, *causer, *receiver, *tag, *modifier;

    StateTracker() {
        reset();
    }

    bool canSet(e_types type, lua_State* state) {
        if (opts == nullptr) return false;
        bool allowed = false;
        switch (type) {
        case e_types::kFlags:
            allowed = set(flags, state);
            break;
        case e_types::kCauser:
            allowed = set(causer, state);
            break;
        case e_types::kReceiver:
            allowed = set(receiver, state);
            break;
        case e_types::kTag:
            allowed = set(tag, state);
            break;
        case e_types::kModifier:
            allowed = set(modifier, state);
            break;
        }
        return allowed;
    }

    void reset() {
        flags = causer = receiver = tag = modifier = nullptr;
    }

private:

    inline bool set(const lua_State*& current, lua_State* next) {
        if (current == nullptr) current = next;
        return current == next;
    }
} flagStates;

namespace odl {
    void set(halo::damage_script_options* opts_) {
        opts = opts_;
        flagStates.reset();
    }

    void reset() {
        opts = nullptr;
    }
}

int l_odl_causer(lua_State* L) {
    halo::ident objid;
    std::tie(objid) = phlua::callback::getArguments<halo::ident>(L, __FUNCTION__);

    if (flagStates.canSet(e_types::kCauser, L)) {
        opts->causer = objid;
        halo::s_player* player = halo::game::getPlayerFromObjectId(opts->causer);
        if (player) opts->causer_player = player->getPlayerIdent();
        else opts->causer_player = halo::ident();
    }
    return 0;
}

int l_odl_receiver(lua_State* L) {
    halo::ident objid;
    std::tie(objid) = phlua::callback::getArguments<halo::ident>(L, __FUNCTION__);

    if (flagStates.canSet(e_types::kReceiver, L)) {
        opts->receiver = objid;
    }
    return 0;
}

int l_odl_tag(lua_State* L) {
    halo::s_tag_entry* tag;
    std::tie(tag) = phlua::callback::getArguments<decltype(tag)>(L, __FUNCTION__);

    if (flagStates.canSet(e_types::kTag, L)) {
        opts->tag = tag->id;
    }
    return 0;
}

int l_odl_multiplier(lua_State* L) {
    float modifier;
    std::tie(modifier) = phlua::callback::getArguments<float>(L, __FUNCTION__);
    if (flagStates.canSet(e_types::kModifier, L)) {
        opts->modifier = modifier;
    }
    return 0;
}

int l_odl_flags(lua_State* L) {
    size_t offset;
    boost::optional<bool> value;

    std::tie(offset, value) = phlua::callback::getArguments<size_t, decltype(value)>(L, __FUNCTION__);

    size_t flag = 1 << offset;

    // suicide bit causes instability.. ie if you set it then nade an environment object.
    if (flag == 0x80) return 0;

    if (!value) { // get current value
        bool val = (opts->flags & flag) == flag;
        return phlua::callback::pushReturns(L, std::make_tuple(val));
    } else if (flagStates.canSet(e_types::kFlags, L)) {
        if (*value)	opts->flags |= flag;
        else opts->flags &= ~flag;
    }
    return 0;
}