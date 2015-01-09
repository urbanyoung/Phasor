#pragma once

#include <vector>
#include <string>
#include <boost/optional.hpp>

/*! \namespace scripting::events
* \brief All script events are raised through functions here.
*
*	All scripting events receive at least one parameter, this parameter
*	determines whether or not your return value will be considered.
*	It is the last parameter on all functions and you don't need to explicitly
*	add it to your parameter list unless you want to use it.
*
*	### CHANGES
*		- If you don't want your return value to be considered you \b must either
*		  return \c nil or return nothing at all.
*		- If you return a value and \c relevant == \c false it will be ignored.
*		- The value indicating the server is executing the command is \c nil not \c -1
*		- OnClientUpdate only receives the player's memory id, not their object id.
*		- OnPlayerJoin only receives the player's memory id, not their team too.
*		- OnWeaponAssignment should return the \c id on the desired weapon, it shouldn't
*		  returned the result of \c lookuptag, instead use \c gettagid. Also, for
*		  performance reasons scripts should save the result of \c gettagid instead
*		  of calling it each time.
*		- OnDamageLookup has completely changed, you shouldn't write to its
*		  \c tagdata anymore. Instead, use the odl_ functions to modify its
*		  behaviour.
*
*/

namespace halo {
    struct ident;
    struct s_player;
    struct s_tag_entry;

    struct s_damage_info;
    struct damage_script_options;
    struct s_hit_info;

    namespace objects {
        struct s_object_creation_disposition;
    }

    namespace server {
        namespace chat {
            enum e_chat_types;
        }
    }
}

namespace scripting {
    namespace events {
        extern const std::vector<std::string> eventList;

        /*! \brief Called when a player has changed team.
        *
        *	\param player The player who is changing team.
        *	\param old_team The player's old team
        *	\param new_team The player's new team
        *	\param relevant Determines whether or not your return value matters.
        *	\return boolean indicating whether or not the player should change team.
        *
        *	\remark
        *	It is important that if you process this function, you process \c relevant
        *	correctly.
        *
        *	Definition:
        *	\code
        *		function OnTeamChange(player, old_team, new_team, relevant)
        *	\endcode
        */
        bool OnTeamChange(const halo::s_player& player, bool relevant, size_t old_team,
                          size_t new_team);

        /*! \brief Called when a server command is being executed.
        *
        * This function is only called when the player enters the correct
        * rcon password and when they are allowed to execute the desired command.
        *
        * \param player The player executing the command
        * \param command The command being executed.
        * \return boolean indicating whether or not Phasor should process the command.
        *
        * Definition:
        * \code
        *		function OnServerCommand(player, command)
        *	\endcode
        */
        bool OnServerCommand(const halo::s_player* player, const std::string& command);

        /*! \brief Called when a player without the correct password is trying to
        *	execute a server command.
        *
        * \param player The player wanting to execute the command
        * \param command The command they want to execute.
        * \return boolean indicating whether or not the command can be processed.
        *
        * \remark
        * Only called when the \b incorrect password is supplied.
        *
        * Definition:
        * \code
        *		function OnServerCommandAttempt(player, command, password)
        *	\endcode
        */
        bool OnServerCommandAttempt(const halo::s_player& player, const std::string& command,
                                    const std::string& password);

        /*! \brief Called when a new game is starting.
        *
        *	\param map The map the game is running.
        *
        *	Definition:
        *	\code
        *		function OnNewGame(map)
        *	\endcode
        */
        void OnNewGame(const std::string& map);

        /*! \brief Called when a game is ending.
        *
        *	\param stage The stage of the game that has ended.
        *
        *  \remark \c stage can be either:
        *			- 1	  The game has just ended, and the ingame score card is shown.
        *			- 2   The postgame scorecard is shown.
        *			- 3   Players can now leave.
        *
        *	Definition:
        *	\code
        *		function OnGameEnd(stage)
        *	\endcode
        */
        void OnGameEnd(size_t stage);

        /*! \brief Called when a player is attempting to join.
        *
        *	\param hash The joining player's hash
        *	\param ip The joining player's ip
        *	\return Boolean indicating whether or not the player is allowed to join.
        *
        *	Definition:
        *	\code
        *		function OnBanCheck(hash, ip)
        *	\endcode
        */
        bool OnBanCheck(const std::string& hash, const std::string& ip);

        /*! \brief Called when a client sends its update packet
        *
        *	\param player The player's memory id.
        *
        *	\remark This function is called 30 times a second for every
        *	player in the server. Do not process this event unless you
        *	absolutely have to, and when processing it make your code as
        *	efficient as possible.
        *
        *	Definition:
        *	\code
        *		function OnClientUpdate(player)
        *	\endcode
        */
        void OnClientUpdate(const halo::s_player& player);

        /*! \brief Called when a player successfully joins the game.
        *
        *	\param player The joining player's memory id.
        *
        *	Definition:
        *	\code
        *		function OnPlayerJoin(player)
        *	\endcode
        */
        void OnPlayerJoin(const halo::s_player& player);

        /*! \brief Called when a player quits.
        *
        *	\param player The player who left.
        *
        *	Definition:
        *	\code
        *		function OnPlayerLeave(player)
        *	\endcode
        */
        void OnPlayerLeave(const halo::s_player& player);

        /*! \brief Called when a player needs to be assigned a team.
        *
        *	\param team The team the player is about to be put on.
        *	\return The team you want the player on, or \c nil if you don't care.
        *
        *	Definition:
        *	\code
        *		function OnTeamDecision(team)
        *	\endcode
        */
        size_t OnTeamDecision(size_t team);

        /*! \brief Called when a player has spawned.
        *
        *	\param player The player who is spawning.
        *	\param objid The player's new object id.
        *
        *	Definition:
        *	\code
        *		function OnPlayerSpawn(player, m_objectId)
        *	\endcode
        */
        void OnPlayerSpawn(const halo::s_player& player, halo::ident m_objectId);

        /*! \brief Called when the server has been notified on the player's spawn.
        *
        *	\param player The player who is spawning.
        *	\param objid The player's new object id.
        *
        *	Definition:
        *	\code
        *		function OnPlayerSpawnEnd(player, m_objectId)
        *	\endcode
        */
        void OnPlayerSpawnEnd(const halo::s_player& player, halo::ident m_objectId);

        /*! \brief Called when an object has just been created. You can modify
        *	most object settings and have it sync.
        *
        *	\param objid The object id of the newly created object.
        *
        *	Definition:
        *	\code
        *		function OnObjectCreation(m_objid)
        *	\endcode
        */
        void OnObjectCreation(halo::ident m_objectId);

        /*! \brief Called when an object wants to be created. You can block it.
        *
        *	\param mapid The id of the object being created.
        *	\param parentid The object id of the to be created object's parent.
        *	\param player The memory id of the owning player.
        *	\return boolean indicating whether or not the object should be created
        *	\b OR map id of object to create instead.
        *
        *	\remark
        *	Both \c parent and \c player can be nil if the object doesn't
        *	have a parent or isn't owned by a player.
        *
        *	\remark
        *	If you return a map id, it must be of the same class as the input
        *	(ie weap) or it is ignored, and the object is created.
        *
        *	Definition:
        *	\code
        *		function OnObjectCreationAttempt(mapid, parentid, player)
        *	\endcode
        */
        boost::optional<halo::ident> OnObjectCreationAttempt(const halo::objects::s_object_creation_disposition* info,
                                                             const halo::s_player* player,
                                                             bool& allow);

        /*! \brief Called when an object is being assigned their spawn weapons.
        *
        *	\param player The memory id of the player, if the weapons belong to a player.
        *	\param owner The object id of the object which owns the assigned weapon.
        *	\param order Which weapon is being assigned (first, second, third etc)
        *	\param weap_id	The map id of the weapon which will be assigned.
        *	\return The map id of the weapon you wish to assign.
        *
        *	\remarks
        *		- \c player is \c nil when assigning weapons to vehicles.
        *		- The value returned should be either:
        *			- \c nil if you don't want to change the assigned weapon.
        *			- \em or The map id of the weapon you wish to assign
        *			- \em or -1 if you don't want the weapon to be assigned.
        *
        *	\remark
        *	I recommend you do all of your tag lookups in \c OnNewGame and
        *	store the results globally.
        *
        *	\remark
        *	For this to have any effect the gametype must have starting equipment
        *	set to generic.
        *
        * Definition:
        * \code
        *		function OnWeaponAssignment(player, owner_id, order, weap_id)
        *	\endcode
        */
        boost::optional<halo::ident> OnWeaponAssignment(const halo::s_player* player,
                                                        halo::ident owner,
                                                        size_t order,
                                                        halo::ident weap_id);

        /*! \brief Called when a player interacts with an object (ie stands on it)
        *
        *	\param player The player interacting with the object
        *	\param objid  The object being interacted with.
        *	\param mapid  The map id of the object being interacted with.
        *	\return Boolean indicating whether or not to allow the interaction.
        *
        *	Definition:
        *	\code
        *		function OnObjectInteraction(player, objid, mapid)
        *	\endcode
        */
        bool OnObjectInteraction(const halo::s_player& player, halo::ident objid,
                                 halo::ident mapid);

        /*! \brief Called when the server needs to apply damage to an object.
        *
        *	\param receiving object id of the object that is to receive the damage.
        *	\param causing object id of the object that is causing the damage.
        *	\param tagid tag id of damage tag which is being used.
        *	\param tagdata Memory address of the tag's meta data.

        *	\return boolean indicating whether or not to allow the damage.
        *
        *	Definition:
        *	\code
        *		function OnDamageLookup(receiving, causing, tagid)
        *	\endcode
        */
        bool OnDamageLookup(halo::s_damage_info* dmg, void* metaData,
                            halo::ident receiver, halo::damage_script_options& out);

        /*! \brief Called when the server is about to apply damage to an object.
        *
        *	\param receiving object id of the object that is to receive the damage.
        *	\param causing object id of the object that is causing the damage.
        *	\param tagid tag id of damage tag which is being used.
        *	\param hit string describing the hit location (eg head, legs)
        *	\param backtap true if damage is a melee from behind, false otherwise.
        *	\return boolean indicating whether or not to allow the damage.
        *
        *	\remark
        *	You cannot modify the amount of damage done because it's already
        *	been calculated. The only action you can take is to block the
        *	damage.
        *
        *	\remark
        *	There are many possible hit strings, including but not limited to:
        *	head, legs, body, metal, rubber, trunk, stock, ...
        *
        *	Definition:
        *	\code
        *		function OnDamageApplication(receiving, causing, tagid, hit, backtap)
        *	\endcode
        */
        bool OnDamageApplication(const halo::s_damage_info* dmg, halo::ident receiver,
                                 const halo::s_hit_info* hit, bool backtap);

        /*! \brief Called when a player chats in the server.
        *
        *	\param player The chatting player's memory id
        *	\param type The type of chat
        *	\param msg The message being sent.
        *	\return allow, msg, type
        *
        *	\remark
        *	\c player is \c nil if it's a server message. If it's a private
        *	server message then \c player is the player who will receive
        *	the message.
        *
        *	\remark
        *	Valid type values:
        *		- 0 All chat
        *		- 1 Team chat
        *		- 2 Vehicle chat
        *		- 3 Server message
        *		- 4 Private server message
        *
        *	\remark
        *	return value: you can include (or not) any of the three return
        *	values. Note: you cannot change a server message into a player
        *	message. Below are some examples:
        *	\code
        *		return false -- block the message
        *	\endcode
        *	\code
        *		return true, "Hello" -- change the message to Hello
        *	\endcode
        *	\code
        *		function OnServerChat(player, type, msg)
        *			if (type == 4) then return end
        *			return true, msg, 1 -- don't change the message, but make it team-only
        *		end
        *	\endcode
        *
        *	Definition:
        *	\code
        *		function OnServerChat(player, type, msg)
        *	\endcode
        *
        */
        bool OnServerChat(const halo::s_player* sender, const std::string& msg,
                          halo::server::chat::e_chat_types& type, std::string& change_msg);

        /*! \brief Called when a player is wanting to enter a vehicle.
        *
        *	\param player The player's memory id.
        *	\param veh_id The object id of the vehicle they're trying to enter.
        *	\param seat The seat they're trying to enter.
        *	\param mapid The map id of the vehicle they're trying to enter.
        *	\param relevant Whether or not you can stop them entering.
        *
        *	\return Boolean indicating whether or not they should be allowed to enter.
        *
        *	Definition:
        *	\code
        *		function OnVehicleEntry(player, veh_id, seat, mapid, relevant)
        *	\endcode
        */
        bool OnVehicleEntry(const halo::s_player& player, halo::ident veh_id,
                            size_t seat, bool relevant);

        /*! \brief Called when a player is leaving a vehicle.
        *
        *	\param player The player's memory id
        *	\param relevant Boolean indicating whether or not you can stop them leaving.
        *	\return Boolean indicating whether or not they can leave.
        *
        *	Definition:
        *	\code
        *		function OnVehicleEject(player, relevant)
        *	\endcode
        */
        bool OnVehicleEject(const halo::s_player& player, bool forceEjected);

        /*! \brief Called when a player is killed.
        *
        *	\param killer The killer's memory id.
        *	\param victim The victim's memory id.
        *	\param mode Describes how they died.
        *
        *	\return Boolean indicating whether or not the kill message should be shown.
        *
        *	\remark
        *		- \c killer can be \c nil.
        *		- \c victim is never \c nil.
        *		- Modes:
        *			- 0 Killed by the server.
        *			- 1 Killed by fall damage.
        *			- 2 Killed by the guardians.
        *			- 3 Killed by a vehicle.
        *			- 4 Killed by \c killer
        *			- 5 Betrayed by \c killer
        *			- 6 Suicide
        *
        *	Definition:
        *	\code
        *		function OnPlayerKill(killer, victim, mode)
        *	\endcode
        */
        bool OnPlayerKill(const halo::s_player& victim, const halo::s_player* killer,
                          size_t mode);

        /*! \brief Called when a player gets a kill streak.
        *
        *	\param player The player's memory id
        *	\param multiplier The multiplier the player got.
        *
        *	\remark
        *	Valid multipliers are:
        *		- 7 Double kill
        *		- 9 Triple kill
        *		- 10 Killtacular
        *		- 11 Killing spree
        *		- 12 Running riot
        *		- 16 Double kill w/ score
        *		- 15 Triple kill w/ score
        *		- 14 Killtacular w/ score
        *		- 18 Killing spree w/ score
        *		- 17 Running riot w/ score
        *
        *	\remark
        *	I think the w/ score ones happen in Slayer.
        *
        *	Definition:
        *	\code
        *		function OnKillMultiplier(player, multiplier)
        *	\endcode
        */
        void OnKillMultiplier(const halo::s_player& player, size_t multiplier);

        /*! \brief Called when a weapon is being reloaded.
        *
        *	\param player The player who is reloading.
        *	\param objid The object id of the weapon being reloaded.
        *	\return Boolean indicating whether or not they can reload.
        *
        *	\remark
        *	\c player can be nil if a vehicle's weapon is being reloaded.
        *
        *	Definition:
        *	\code
        *		function OnWeaponReload(player, objid)
        *	\endcode
        */
        bool OnWeaponReload(const halo::s_player* player, halo::ident weap);

        /*! \brief Called when a player is requesting a certain name (during connection).
        *
        *	\param hash The hash of the requesting machine.
        *	\param name The requested name.
        *	\return allow, new_name
        *
        *	\remark
        *	If \c allow is \c false then the player is assigned a random
        *	name. If \c new_name is specified, then \c allow is ignored and
        *	the player's name is changed to \c new_name
        *
        * Definition:
        * \code
        *		function OnNameRequest(hash, name)
        *	\endcode
        */
        bool OnNameRequest(const std::string& hash,
                           const std::string& name,
                           boost::optional<std::string>& changeTo);

        /*!
        \brief Called when a hash is validated by gamespy

        \param hash The hash that was checked
        \param status Result of the check (1 = valid, 2 = invalid, 3 = valid hash but invalid challenge)

        \remark
        Status 2 indicates that somebody is trying to spoof a player's hash
        (i.e. pretend to be them). The player is immediately kicked after
        calling this function.

        \remark
        This function is only called when sv_public is 1
        */
        void OnHashValidation(const std::string& hash,
                              int status);
    }
}
