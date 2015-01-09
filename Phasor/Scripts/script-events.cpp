#include "script-events.h"
#include "scripting.hpp"
#include "../Phasor/Globals.h"
#include "../Phasor/Halo/Game/Damage.h"
#include "../Phasor/Halo/Server/Chat.h"
#include "PhasorAPI/damagelookup.h"

namespace scripting {
    namespace events {
        const std::vector<std::string> eventList
        {
        "OnScriptUnload",
        "OnTeamChange",
        "OnServerCommand",
        "OnServerCommandAttempt",
        "OnNewGame",
        "OnGameEnd",
        "OnBanCheck",
        "OnClientUpdate",
        "OnPlayerJoin",
        "OnPlayerLeave",
        "OnTeamDecision",
        "OnPlayerSpawn",
        "OnPlayerSpawnEnd",
        "OnObjectCreation",
        "OnObjectCreationAttempt",
        "OnWeaponAssignment",
        "OnObjectInteraction",
        "OnDamageLookup",
        "OnDamageApplication",
        "OnServerChat",
        "OnVehicleEntry",
        "OnVehicleEject",
        "OnPlayerKill",
        "OnKillMultiplier",
        "OnWeaponReload",
        "OnNameRequest"
        };

        inline bool default_true(const boost::optional<std::tuple<bool>>& allow) {
            return allow ? std::get<0>(*allow) : true;
        }

        inline bool default_false(const boost::optional<std::tuple<bool>>& allow) {
            return allow ? std::get<0>(*allow) : false;
        }

        bool OnTeamChange(const halo::s_player& player, bool relevant, size_t old_team,
                          size_t new_team)
        {
            auto allow = scripting::Caller<bool>::call(*g_Scripts, "OnTeamChange", relevant,
                                                       std::make_tuple(std::cref(player), old_team, new_team));
            return !relevant ? true : default_true(allow);
        }

        bool OnServerCommand(const halo::s_player* player, const std::string& command)
        {
            auto allow = scripting::Caller<bool>::call(*g_Scripts, "OnServerCommand",
                                                       std::make_tuple(player, std::cref(command)));
            return default_true(allow);
        }

        bool OnServerCommandAttempt(const halo::s_player& player, const std::string& command,
                                    const std::string& password)
        {
            auto allow = scripting::Caller<bool>::call(*g_Scripts, "OnServerCommandAttempt",
                                                       std::make_tuple(std::cref(player),
                                                       std::cref(command),
                                                       std::cref(password)));
            return default_false(allow);
        }

        void OnNewGame(const std::string& map)
        {
            scripting::Caller<>::call(*g_Scripts, "OnNewGame",
                                      std::make_tuple(std::cref(map)));
        }

        void OnGameEnd(size_t stage)
        {
            scripting::Caller<>::call(*g_Scripts, "OnGameEnd",
                                      std::make_tuple(stage));
        }

        /*! \todo make function so scripts can check if hash-checking is on */
        bool OnBanCheck(const std::string& hash, const std::string& ip)
        {
            auto allow = scripting::Caller<bool>::call(*g_Scripts, "OnBanCheck",
                                                       std::make_tuple(std::cref(hash),
                                                       std::cref(ip)));
            return default_true(allow);
        }

        void OnClientUpdate(const halo::s_player& player)
        {
            scripting::Caller<>::call(*g_Scripts, "OnClientUpdate",
                                      std::make_tuple(std::cref(player)));
        }

        void OnPlayerJoin(const halo::s_player& player)
        {
            scripting::Caller<>::call(*g_Scripts, "OnPlayerJoin",
                                      std::make_tuple(std::cref(player)));
        }

        void OnPlayerLeave(const halo::s_player& player)
        {
            scripting::Caller<>::call(*g_Scripts, "OnPlayerLeave",
                                      std::make_tuple(std::cref(player)));
        }

        size_t OnTeamDecision(size_t team)
        {
            auto result = scripting::Caller<size_t>::call(*g_Scripts, "OnTeamDecision",
                                                          std::make_tuple(team));
            return result ? std::get<0>(*result) : team;
        }

        void OnPlayerSpawn(const halo::s_player& player, halo::ident m_objectId)
        {
            scripting::Caller<>::call(*g_Scripts, "OnPlayerSpawn",
                                      std::make_tuple(std::cref(player), m_objectId));
        }

        void OnPlayerSpawnEnd(const halo::s_player& player, halo::ident m_objectId)
        {
            scripting::Caller<>::call(*g_Scripts, "OnPlayerSpawnEnd",
                                      std::make_tuple(std::cref(player), m_objectId));
        }

        void OnObjectCreation(halo::ident m_objectId)
        {
            scripting::Caller<>::call(*g_Scripts, "OnObjectCreation",
                                      std::make_tuple(m_objectId));
        }

        boost::optional<halo::ident> OnObjectCreationAttempt(const halo::objects::s_object_creation_disposition* info,
                                                             const halo::s_player* player,
                                                             bool& allow)
        {
            boost::optional<std::tuple<size_t>> result;

            if (player != nullptr) {
                result = scripting::Caller<size_t>::call(*g_Scripts, "OnObjectCreationAttempt",
                                                          std::make_tuple(info->map_id, info->parent, player->memory_id));                
            } else if (info->player_ident.valid()) {
                result = scripting::Caller<size_t>::call(*g_Scripts, "OnObjectCreationAttempt",
                                                          std::make_tuple(info->map_id, info->parent, info->player_ident.slot));                
            } else {
                result = scripting::Caller<size_t>::call(*g_Scripts, "OnObjectCreationAttempt",
                                                         std::make_tuple(info->map_id, info->parent, lua::types::Nil()));
            }

            if (!result) return boost::none;
            size_t x = std::get<0>(*result);
            if (x == 0 || x == 1) {
                allow = x == 1;
                return boost::none;
            }
            return boost::optional<halo::ident>(halo::make_ident(x));
        }

        boost::optional<halo::ident> OnWeaponAssignment(const halo::s_player* player,
                                                        halo::ident owner,
                                                        size_t order,
                                                        halo::ident weap_id)
        {
            auto result = scripting::Caller<halo::ident>::call(*g_Scripts, "OnWeaponAssignment",
                                                               std::make_tuple(player,
                                                               owner, order, weap_id));

            boost::optional<halo::ident> changeId;
            if (result) changeId = std::get<0>(*result);

            return changeId;
        }

        bool OnObjectInteraction(const halo::s_player& player, halo::ident objid,
                                 halo::ident mapid)
        {
            auto allow = scripting::Caller<bool>::call(*g_Scripts, "OnObjectInteraction",
                                                       std::make_tuple(std::cref(player),
                                                       objid, mapid));
            return default_true(allow);
        }

        bool OnDamageLookup(halo::s_damage_info* dmg, void* metaData,
                            halo::ident receiver, halo::damage_script_options& out)
        {
            odl::set(&out);

            auto allow = scripting::Caller<bool>::call(*g_Scripts, "OnDamageLookup",
                                                       std::make_tuple(receiver,
                                                       dmg->causer, dmg->tag_id,
                                                       (size_t)metaData));
            odl::reset();
            return default_true(allow);
        }

        bool OnDamageApplication(const halo::s_damage_info* dmg, halo::ident receiver,
                                 const halo::s_hit_info* hit, bool backtap)
        {
            auto allow = scripting::Caller<bool>::call(*g_Scripts, "OnDamageApplication",
                                                       std::make_tuple(receiver,
                                                       dmg->causer, dmg->tag_id,
                                                       hit->desc, backtap));
            return default_true(allow);
        }

        bool OnServerChat(const halo::s_player* sender, const std::string& msg,
                          halo::server::chat::e_chat_types& type, std::string& change_msg)
        {
            bool allow;
            boost::optional<std::string> newMsg;
            boost::optional<size_t> newType;

            size_t type_as_num = (size_t)type;

            auto result = scripting::Caller<bool,
                decltype(newMsg),
                decltype(newType)>::call(*g_Scripts, "OnServerChat",
                std::make_tuple(sender, type_as_num, std::cref(msg)));

            if (!result) return true;

            allow = std::get<0>(*result);
            newMsg = std::get<1>(*result);
            newType = std::get<2>(*result);

            if (newMsg) change_msg = *newMsg;
            if (newType && type != halo::server::chat::e_chat_types::kChatServer) {
                using namespace halo::server::chat;
                size_t chat_type = (size_t)*newType;
                // can only change type of non-server chat
                if (chat_type >= (size_t)e_chat_types::kChatAll &&
                    chat_type < (size_t)e_chat_types::kChatPrivate)
                {
                    type = (e_chat_types)chat_type;
                }
            }

            return allow;
        }

        bool OnVehicleEntry(const halo::s_player& player, halo::ident veh_id,
                            size_t seat, bool relevant)
        {
            halo::objects::s_halo_object* obj = (halo::objects::s_halo_object*)
                halo::objects::GetObjectAddress(veh_id);
            if (!obj) return true;

            auto allow = scripting::Caller<bool>::call(*g_Scripts, "OnVehicleEntry", relevant,
                                                       std::make_tuple(std::cref(player), veh_id,
                                                       seat, obj->map_id));
            return !relevant ? true : default_true(allow);
        }

        bool OnVehicleEject(const halo::s_player& player, bool forceEjected)
        {
            auto allow = scripting::Caller<bool>::call(*g_Scripts, "OnVehicleEject", forceEjected,
                                                       std::make_tuple(std::cref(player)));
            return default_true(allow);
        }

        bool OnPlayerKill(const halo::s_player& victim, const halo::s_player* killer,
                          size_t mode)
        {
            auto allow = scripting::Caller<bool>::call(*g_Scripts, "OnPlayerKill",
                                                       std::make_tuple(killer, std::cref(victim),
                                                       mode));
            return default_true(allow);
        }

        void OnKillMultiplier(const halo::s_player& player, size_t multiplier)
        {
            scripting::Caller<>::call(*g_Scripts, "OnKillMultiplier",
                                      std::make_tuple(std::cref(player), multiplier));
        }

        bool OnWeaponReload(const halo::s_player* player, halo::ident weap)
        {
            auto allow = scripting::Caller<bool>::call(*g_Scripts, "OnWeaponReload",
                                                       std::make_tuple(player, weap));
            return default_true(allow);
        }

        bool OnNameRequest(const std::string& hash,
                           const std::string& name,
                           boost::optional<std::string>& changeTo)
        {
            boost::optional<std::tuple<
                boost::optional<bool>,
                boost::optional<std::string>>> result;

            result = scripting::Caller<
                boost::optional<bool>,
                boost::optional<std::string>>::call(*g_Scripts, "OnNameRequest",
                std::make_tuple(std::cref(hash), std::cref(name)));

            if (result) {
                auto allow = std::get<0>(*result);
                auto name = std::get<1>(*result);
                if (name) {
                    changeTo = std::move(name);
                    return true;
                } else if (allow) {
                    return *allow;
                }
            }

            return true;
        }

        void OnHashValidation(const std::string& hash,
                              int status)
        {
            scripting::Caller<>::call(*g_Scripts, "OnHashValidation",
                                      std::make_tuple(std::cref(hash), status));
        }
    }
}