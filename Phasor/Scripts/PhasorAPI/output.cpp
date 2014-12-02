#include "output.h"
#include "../phasor-lua.hpp"
#include "../../Phasor/Globals.h"
#include "../../Phasor/Halo/Server/Server.h"

void writeToStream(const phlua::ProcessedString& str, COutStream& stream) {
    for (auto itr = str.msgs.cbegin(); itr != str.msgs.cend(); ++itr)
        stream << *itr << endl;
}

int l_hprintf(lua_State* L) {
    phlua::ProcessedString str;
    boost::optional<halo::s_player*> player; // backwards compatibility

    std::tie(str, player) = phlua::callback::getArguments<decltype(str), decltype(player)>(L, __FUNCTION__);

    if (player) writeToStream(str, *(*player)->console_stream);
    else writeToStream(str, *g_PrintStream);
    return 0;
}

int l_sendconsoletext(lua_State* L) {
    halo::s_player* player;
    phlua::ProcessedString str;

    std::tie(player, str) = phlua::callback::getArguments<decltype(player), decltype(str)>(L, __FUNCTION__);

    writeToStream(str, *player->console_stream);
    return 0;
}

int l_privatesay(lua_State* L) {
    halo::s_player* player;
    phlua::ProcessedString str;
    boost::optional<bool> prepend;

    std::tie(player, str, prepend) = phlua::callback::getArguments<decltype(player), decltype(str), decltype(prepend)>(L, __FUNCTION__);

    if (!prepend) prepend = true;

    halo::PlayerChatStreamRaw raw(*player);
    writeToStream(str, *prepend ? *player->chat_stream : raw);
    return 0;
}

int l_say(lua_State* L) {
    phlua::ProcessedString str;
    boost::optional<bool> prepend;

    std::tie(str, prepend) = phlua::callback::getArguments<decltype(str), decltype(prepend)>(L, __FUNCTION__);

    if (!prepend) prepend = true;
    writeToStream(str, *prepend ? halo::server::say_stream : halo::server::say_stream_raw);
    return 0;
}

int l_respond(lua_State* L) {
    phlua::ProcessedString str;
    std::tie(str) = phlua::callback::getArguments<decltype(str)>(L, __FUNCTION__);

    halo::s_player* player = halo::server::GetPlayerExecutingCommand();
    COutStream& stream = (player == NULL) ?
        static_cast<COutStream&>(*g_PrintStream) :
        static_cast<COutStream&>(*player->console_stream);
    writeToStream(str, stream);
    return 0;
}

int l_log_msg(lua_State* L) {
    size_t logId;
    phlua::ProcessedString str;
    std::tie(logId, str) = phlua::callback::getArguments<size_t, decltype(str)>(L, __FUNCTION__);

    COutStream* stream = nullptr;
    switch (logId) {
    case 1: // game log
        for (auto itr = str.msgs.cbegin(); itr != str.msgs.cend(); ++itr)
            g_GameLog->WriteLog(kScriptEvent, L"%s", itr->c_str());
        break;
    case 2: //phasor log
        stream = g_PhasorLog.get();
        break;
    case 3: // rcon log
        stream = g_RconLog.get();
        break;
    case 4: // script log
        stream = g_ScriptsLog.get();
        break;
    default:
        luaL_argerror(L, 1, "invalid log id");
        break;
    }

    if (stream)
        writeToStream(str, *stream);
    return 0;
}