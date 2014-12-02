#include "haloobjects.h"
#include "../phasor-lua.hpp"
#include "../../Phasor/Halo/Game/Objects.h"
#include "../../Phasor/Halo/Game/Damage.h"

int l_getobject(lua_State* L) {
    boost::optional<halo::objects::s_halo_object*> object;
    std::tie(object) = phlua::callback::getArguments<decltype(object)>(L, __FUNCTION__);
    return phlua::callback::pushReturns(L, std::make_tuple(object));
}

int l_getobjectcoords(lua_State* L) {
    halo::objects::s_halo_object* object;
    std::tie(object) = phlua::callback::getArguments<decltype(object)>(L, __FUNCTION__);
    if (object->vehicleId.valid()) {
        halo::objects::s_halo_object* vehi = (halo::objects::s_halo_object*)halo::objects::GetObjectAddress(object->vehicleId);
        if (vehi) object = vehi;
    }

    return phlua::callback::pushReturns(L, std::make_tuple(std::cref(object->location)));
}

int l_objectaddrtoplayer(lua_State* L) {
    unsigned long addr;
    std::tie(addr) = phlua::callback::getArguments<unsigned long>(L, __FUNCTION__);

    boost::optional<int> playerId;
    for (int i = 0; i < 16; i++) {
        halo::s_player* player = halo::game::getPlayer(i);
        if (player && halo::objects::GetObjectAddress(player->mem->object_id) == (void*)addr) {
            playerId = i;
            break;
        }
    }

    return phlua::callback::pushReturns(L, std::make_tuple(playerId));
}

int l_objectidtoplayer(lua_State* L) {
    boost::optional<halo::ident> objId;
    std::tie(objId) = phlua::callback::getArguments<decltype(objId)>(L, __FUNCTION__);

    boost::optional<int> playerId;
    if (objId) {
        for (int i = 0; i < 16; i++) {
            halo::s_player* player = halo::game::getPlayer(i);
            if (player && player->mem->object_id == objId) {
                playerId = i;
                break;
            }
        }
    }

    return phlua::callback::pushReturns(L, std::make_tuple(playerId));
}

int l_createobject(lua_State* L) {
    halo::s_tag_entry* tag;
    halo::ident parentId;
    boost::optional<int> respawnTime;
    bool do_respawn;
    vect3d pos;

    std::tie(tag, parentId, respawnTime, do_respawn, pos.x, pos.y, pos.z) =
        phlua::callback::getArguments
        <
        decltype(tag),
        decltype(parentId),
        decltype(respawnTime),
        decltype(do_respawn),
        decltype(pos.x),
        decltype(pos.y),
        decltype(pos.z)
        >(L, __FUNCTION__);

    if (parentId == 0) parentId = halo::ident();

    halo::ident objid;
    bool success = halo::objects::CreateObject(tag, parentId, respawnTime, do_respawn, &pos, objid);

    if (success) {
        return phlua::callback::pushReturns(L, std::make_tuple(objid));
    } else {
        return 0;
    }
}

int l_destroyobject(lua_State* L) {
    halo::ident objid;
    std::tie(objid) = phlua::callback::getArguments<halo::ident>(L, __FUNCTION__);
    if (!halo::objects::DestroyObject(objid))
        return luaL_argerror(L, 1, "invalid object id.");
    return 0;
}

int l_assignweapon(lua_State* L) {
    halo::s_player* player;
    halo::ident weaponId;
    std::tie(player, weaponId) = phlua::callback::getArguments<halo::s_player*, halo::ident>(L, __FUNCTION__);

    bool success = halo::objects::AssignPlayerWeapon(*player, weaponId);
    return phlua::callback::pushReturns(L, std::make_tuple(success));
}

int l_entervehicle(lua_State* L) {
    halo::s_player* player;
    halo::ident vehicleId;
    int seat;

    std::tie(player, vehicleId, seat) =
        phlua::callback::getArguments
        <
        decltype(player),
        decltype(vehicleId),
        decltype(seat)
        >(L, __FUNCTION__);

    if (!halo::objects::EnterVehicle(*player, vehicleId, seat)) {
        auto f = boost::format("invalid vehicle id %08x (either bad object or not a vehicle)") % vehicleId;
        return luaL_argerror(L, 2, f.str().c_str());
    }
    return 0;
}

int l_isinvehicle(lua_State* L) {
    halo::s_player* player;
    std::tie(player) = phlua::callback::getArguments<halo::s_player*>(L, __FUNCTION__);
    bool inVehicle = player->InVehicle();
    return phlua::callback::pushReturns(L, std::make_tuple(inVehicle));
}

int l_exitvehicle(lua_State* L) {
    halo::s_player* player;
    std::tie(player) = phlua::callback::getArguments<halo::s_player*>(L, __FUNCTION__);

    halo::objects::ExitVehicle(*player);
    return 0;
}

int l_movobjectcoords(lua_State* L) {
    halo::objects::s_halo_object* obj;
    vect3d newPos;
    std::tie(obj, newPos.x, newPos.y, newPos.z) = phlua::callback::getArguments<decltype(obj), float, float, float>(L, __FUNCTION__);
    if (obj->vehicleId.valid()) {
        halo::objects::s_halo_object* vehi = (halo::objects::s_halo_object*)halo::objects::GetObjectAddress(obj->vehicleId);
        if (vehi) obj = vehi;
    }

    MoveObject(*obj, newPos);
    return 0;
}

int l_gettagid(lua_State* L) {
    std::string tagType, tagName;
    std::tie(tagType, tagName) = phlua::callback::getArguments<std::string, std::string>(L, __FUNCTION__);

    boost::optional<halo::ident> tagId;
    halo::s_tag_entry* tag = halo::LookupTag(halo::s_tag_type(tagType.c_str()), tagName);
    if (tag) tagId = tag->id;
    return phlua::callback::pushReturns(L, std::make_tuple(tagId));
}

int l_gettaginfo(lua_State* L) {
    halo::s_tag_entry* tag;
    std::tie(tag) = phlua::callback::getArguments<decltype(tag)>(L, __FUNCTION__);

    char tagType[5];
    tag->tagType.GetString(tagType);

    return phlua::callback::pushReturns(L, std::make_tuple(tag->tagName, tagType));
}

int l_gettagaddress(lua_State* L) {
    boost::optional<halo::s_tag_entry*> tag;
    std::tie(tag) = phlua::callback::getArguments<decltype(tag)>(L, __FUNCTION__);
    return phlua::callback::pushReturns(L, std::make_tuple(tag));
}

int l_applydmg(lua_State* L) {
    halo::ident receiver;
    float multiplier;
    boost::optional<halo::ident> causer;
    boost::optional<int> flags;

    std::tie(receiver, multiplier, causer, flags) =
        phlua::callback::getArguments
        <
        decltype(receiver),
        decltype(multiplier),
        decltype(causer),
        decltype(flags)
        >(L, __FUNCTION__);

    if (!flags) flags = 0;
    if (!causer) causer = halo::ident();

    bool success = halo::ApplyDamage(receiver, *causer, multiplier, *flags);
    return phlua::callback::pushReturns(L, std::make_tuple(success));
}

int l_applydmgtag(lua_State* L) {
    halo::ident receiver;
    halo::s_tag_entry* tag;
    boost::optional<float> multiplier;
    boost::optional<halo::ident> causer;
    boost::optional<int> flags;

    std::tie(receiver, tag, multiplier, causer, flags) =
        phlua::callback::getArguments
        <
        decltype(receiver),
        decltype(tag),
        decltype(multiplier),
        decltype(causer),
        decltype(flags)
        >(L, __FUNCTION__);

    if (tag->tagType != halo::TAG_JPT) {
        auto f = boost::format("tag %08X is not a damage tag.") % tag->id;
        return luaL_argerror(L, 2, f.str().c_str());
    }

    if (!multiplier) multiplier = 1.0f;
    if (!flags) flags = 0;
    if (!causer) causer = halo::ident();

    halo::ApplyDamage(receiver, *causer, *multiplier, *flags);
    return 0;
}

int l_halointersect(lua_State* L) {
    float dist;
    halo::objects::view_vector view;
    boost::optional<halo::ident> ignoreObj;

    std::tie(dist, view.pos.x, view.pos.y, view.pos.z, view.dir.x, view.dir.y, view.dir.z, ignoreObj) =
        phlua::callback::getArguments
        <
        decltype(dist),
        float, float, float,
        float, float, float,
        decltype(ignoreObj)
        >(L, __FUNCTION__);

    view.dir *= dist;
    if (!ignoreObj) ignoreObj = halo::ident();

    vect3d hit;
    hit.x = 0; hit.y = 0; hit.z = 0;
    halo::ident obj;
    bool intersected = halo::objects::FindIntersection(view, *ignoreObj, hit, obj);

    if (obj.valid())
        return phlua::callback::pushReturns(L, std::make_tuple(intersected, std::cref(hit), obj));
    else
        return phlua::callback::pushReturns(L, std::make_tuple(intersected, std::cref(hit), lua::types::Nil()));
}