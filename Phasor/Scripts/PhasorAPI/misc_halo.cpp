#include "misc_halo.h"
#include "../phasor-lua.hpp"
#include "../../Phasor/Halo/Game/Objects.h"
#include "../../Phasor/Globals.h"
#include "../../Phasor/Directory.h"
#include "../../Phasor/Halo/Server/Server.h"

int l_changeteam(lua_State* L) {
    halo::s_player* player;
    bool forcekill;
    boost::optional<char> team;

    std::tie(player, forcekill, team) = phlua::callback::getArguments<decltype(player), bool, decltype(team)>(L, __FUNCTION__);

    if (!team) team = !player->mem->team;
    player->ChangeTeam(*team, forcekill);
    return 0;
}

int l_kill(lua_State* L) {
    halo::s_player* player;
    std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
    player->Kill();
    return 0;
}

int l_applycamo(lua_State* L) {
    halo::s_player* player;
    float duration;
    std::tie(player, duration) = phlua::callback::getArguments<decltype(player), float>(L, __FUNCTION__);
    player->ApplyCamo(duration);
    return 0;
}

int l_setspeed(lua_State* L) {
    halo::s_player* player;
    float speed;
    std::tie(player, speed) = phlua::callback::getArguments<decltype(player), float>(L, __FUNCTION__);
    player->SetSpeed(speed);
    return 0;
}

int l_updateammo(lua_State* L) {
    using namespace halo::objects;
    halo::ident weaponId;

    std::tie(weaponId) = phlua::callback::getArguments<halo::ident>(L, __FUNCTION__);
    s_halo_weapon* weapon = (s_halo_weapon*)GetObjectAddress(weaponId);
    if (!weapon) luaL_argerror(L, 1, "invalid weapon id");
    weapon->SyncAmmo(weaponId);
    return 0;
}

int l_setammo(lua_State* L) {
    using namespace halo::objects;
    halo::ident weaponId;
    unsigned short clipAmmo, packAmmo;

    std::tie(weaponId, clipAmmo, packAmmo) = phlua::callback::getArguments<halo::ident, unsigned short, unsigned short>(L, __FUNCTION__);
    s_halo_weapon* weapon = (s_halo_weapon*)GetObjectAddress(weaponId);
    if (!weapon) luaL_argerror(L, 1, "invalid weapon id");
    weapon->SetAmmo(packAmmo, clipAmmo);
    weapon->SyncAmmo(weaponId);
    return 0;
}

int l_getprofilepath(lua_State* L) {
    phlua::callback::getArguments<>(L, __FUNCTION__);
    return phlua::callback::pushReturns(L, std::make_tuple(g_ProfileDirectory));
}

int l_getservername(lua_State* L) {
    phlua::callback::getArguments<>(L, __FUNCTION__);
    halo::server::s_server_info* server = halo::server::GetServerStruct();
    return phlua::callback::pushReturns(L, std::make_tuple(server->server_name));
}

// -------------------------------------------------------

int checkAndExecuteCommand(lua_State* L,
                           const std::string& cmd,
                           halo::s_player* player,
                           boost::optional<bool>& wantResult,
                           COutStream& stream)
{
    std::vector<std::string> tokens = TokenizeArgs(cmd);
    if (!tokens.size()) return 0;
    if (tokens[0] == "sv_script_reload")
        luaL_argerror(L, 1, "scripts cannot execute 'sv_script_reload'");

    if (!wantResult || !*wantResult) {
        halo::server::ExecuteServerCommand(cmd, player);
        return 0;
    }

    // Don't like this...
    RecordStream record;
    NotifyStream _(stream, record);

    halo::server::ExecuteServerCommand(cmd, player);

    return phlua::callback::pushReturns(L, std::make_tuple(record.getRecord()));
}

int l_svcmd(lua_State* L) {
    std::string cmd;
    boost::optional<bool> wantResult;

    std::tie(cmd, wantResult) = phlua::callback::getArguments<std::string, decltype(wantResult)>(L, __FUNCTION__);

    return checkAndExecuteCommand(L, cmd, nullptr, wantResult, *g_PrintStream);
}

int l_svcmdplayer(lua_State* L) {
    std::string cmd;
    halo::s_player* player;
    boost::optional<bool> wantResult;

    std::tie(cmd, player, wantResult) = phlua::callback::getArguments<std::string, decltype(player), decltype(wantResult)>(L, __FUNCTION__);

    return checkAndExecuteCommand(L, cmd, player, wantResult, *player->console_stream);
}