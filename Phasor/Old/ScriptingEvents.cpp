#include "ScriptingEvents.h"
#include "Scripting.h"
#include "../Phasor/Halo/Player.h"
#include "../Phasor/Halo/tags.h"
#include "../Phasor/Halo/Game/Damage.h"
#include "PhasorAPI/damagelookup.h"
#include "CallHelper.h"
#include "../Phasor/Halo/Server/Chat.h"

// stupid enum warning
#pragma warning( disable : 4482)

namespace scripting { namespace events {

	static const std::string events[] = 
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

	const std::string* GetEventTable() { return events;	}
	size_t GetEventTableElementCount() { return sizeof(events) / sizeof(events[0]); }

	bool OnTeamChange(const halo::s_player& player, bool relevant, DWORD old_team,
		DWORD new_team)
	{
		PhasorCaller caller;
		// if we're not going to process return values, let scripts know
		if (!relevant) caller.ReturnValueIgnored();
		AddPlayerArg(&player, caller);
		caller.AddArg(old_team);
		caller.AddArg(new_team);
		return HandleResult<bool>(caller.Call("OnTeamChange", result_bool), true);
	}

	bool OnServerCommand(const halo::s_player* player, const std::string& command)
	{
		PhasorCaller caller;
		AddPlayerArg(player, caller);
		caller.AddArg(command);
		return HandleResult<bool>(caller.Call("OnServerCommand", result_bool), true);
	}

	bool OnServerCommandAttempt(const halo::s_player& player, const std::string& command,
		const std::string& password)
	{
		PhasorCaller caller;
		AddPlayerArg(&player, caller);
		caller.AddArg(command);
		caller.AddArg(password);
		return HandleResult<bool>(caller.Call("OnServerCommandAttempt", result_bool), false);
	}

	void OnNewGame(const std::string& map)
	{
		PhasorCaller caller;
		caller.AddArg(map);
		caller.Call("OnNewGame");
	}

	void OnGameEnd(DWORD stage)
	{
		PhasorCaller caller;
		caller.AddArg(stage);
		caller.Call("OnGameEnd");
	}

	/*! \todo make function so scripts can check if hash-checking is on */
	bool OnBanCheck(const std::string& hash, const std::string& ip)
	{
		PhasorCaller caller;
		caller.AddArg(hash);
		caller.AddArg(ip);
		return HandleResult<bool>(caller.Call("OnBanCheck", result_bool), true);
	}

	void OnClientUpdate(const halo::s_player& player)
	{
		PhasorCaller caller;
		AddPlayerArg(&player, caller);
		caller.Call("OnClientUpdate");
	}

	void OnPlayerJoin(const halo::s_player& player)
	{
		PhasorCaller caller;
		AddPlayerArg(&player, caller);
		caller.Call("OnPlayerJoin");
	}

	void OnPlayerLeave(const halo::s_player& player)
	{
		PhasorCaller caller;
		AddPlayerArg(&player, caller);
		caller.Call("OnPlayerLeave");
	}

	bool OnTeamDecision(DWORD in_team, DWORD& out_team)
	{
		PhasorCaller caller;
		caller.AddArg(in_team);
		Result r = caller.Call("OnTeamDecision", result_number);
		if (r.size()) out_team = (DWORD)r.ReadNumber(0).GetValue();
		return r.size() != 0;
	}

	void OnPlayerSpawn(const halo::s_player& player, halo::ident m_objectId)
	{
		PhasorCaller caller;
		AddPlayerArg(&player, caller);
		AddArgIdent(m_objectId, caller);
		caller.Call("OnPlayerSpawn");
	}

	void OnPlayerSpawnEnd(const halo::s_player& player, halo::ident m_objectId)
	{
		PhasorCaller caller;
		AddPlayerArg(&player, caller);
		AddArgIdent(m_objectId, caller);
		caller.Call("OnPlayerSpawnEnd");
	}

	void OnObjectCreation(halo::ident m_objectId)
	{
		PhasorCaller caller;
		AddArgIdent(m_objectId, caller);
		caller.Call("OnObjectCreation");
	}

	e_ident_or_bool OnObjectCreationAttempt(const halo::objects::s_object_creation_disposition* info,
		halo::ident& change_id, bool& allow)
	{
		PhasorCaller caller;
		AddArgIdent(info->map_id, caller);
		AddArgIdent(info->parent, caller);
		if (info->player_ident.valid()) caller.AddArg((DWORD)info->player_ident.slot);
		else caller.AddArgNil();

		e_ident_bool_empty r =  HandleResultIdentOrBool(
			caller.Call("OnObjectCreationAttempt", result_number),
			change_id, allow);

		if (r == e_ident_bool_empty::kEmptySet){
			allow = true;
			return e_ident_or_bool::kBool;
		} else if (r == e_ident_bool_empty::kBoolSet) {
			return e_ident_or_bool::kBool;
		} else {
			return e_ident_or_bool::kIdent;
		}
	}

	bool OnWeaponAssignment(halo::s_player* player, halo::ident owner, DWORD order,
		halo::ident weap_id, halo::ident& out)
	{
		PhasorCaller caller;
		AddPlayerArg(player, caller);
		AddArgIdent(owner, caller);
		caller.AddArg(order);
		AddArgIdent(weap_id, caller); 
		Result r = caller.Call("OnWeaponAssignment", result_number);
		if (!r.size()) return false; // no results
		out = halo::make_ident((unsigned long)r.ReadNumber(0).GetValue());
		return true;
	}

	bool OnObjectInteraction(halo::s_player& player, halo::ident objid,
		halo::ident mapid)
	{
		PhasorCaller caller;
		AddPlayerArg(&player, caller);
		AddArgIdent(objid, caller);
		AddArgIdent(mapid, caller);
		return HandleResult<bool>(caller.Call("OnObjectInteraction", result_bool), true);
	}

	bool OnDamageLookup(halo::s_damage_info* dmg, void* metaData,
		halo::ident receiver, halo::damage_script_options& out)
	{
		odl::setData(&out, dmg, receiver);
		PhasorCaller caller;
		AddArgIdent(receiver, caller);
		AddArgIdent(dmg->causer, caller);
		AddArgIdent(dmg->tag_id, caller);
		caller.AddArg((DWORD)metaData);
		bool b = HandleResult<bool>(caller.Call("OnDamageLookup", result_bool), true);
		odl::reset();
		return b;
	}

	bool OnDamageApplication(const halo::s_damage_info* dmg, halo::ident receiver, 
		const halo::s_hit_info* hit, bool backtap)
	{
		PhasorCaller caller;
		AddArgIdent(receiver, caller);
		AddArgIdent(dmg->causer, caller);
		AddArgIdent(dmg->tag_id, caller);
		caller.AddArg(std::string(hit->desc));
		caller.AddArg(backtap);
		return HandleResult<bool>(caller.Call("OnDamageApplication", result_bool), true);
	}

	bool OnServerChat(const halo::s_player* sender, const std::string& msg,
		halo::server::chat::e_chat_types& type, std::string& change_msg)
	{
		using namespace Common;
		static const scripting::results_t expected = {TYPE_BOOL, TYPE_STRING, TYPE_NUMBER};
		
		PhasorCaller caller;
		AddPlayerArg(sender, caller);
		caller.AddArg(type);
		caller.AddArg(msg);
		Result r = caller.Call("OnServerChat", expected);
		if (r.size() == 0) return true; // no scripts returned anything, use defualt
		if (r.size() > 1) change_msg = r.ReadString(1).GetValue();
		if (r.size() > 2 && type != halo::server::chat::e_chat_types::kChatServer) {
			using namespace halo::server::chat;
			int chat_type = (int)r.ReadNumber(2).GetValue();
			if (chat_type >= (int)e_chat_types::kChatAll && 
				chat_type < (int)e_chat_types::kChatPrivate)
				type = (e_chat_types)chat_type;
		}
		return r.ReadBool(0).GetValue();
	}

	bool OnVehicleEntry(const halo::s_player& player, halo::ident veh_id,
		DWORD seat, bool relevant)
	{
		halo::objects::s_halo_object* obj = (halo::objects::s_halo_object*)
			halo::objects::GetObjectAddress(veh_id);
		if (!obj) return true;

		PhasorCaller caller;
		AddPlayerArg(&player, caller);
		AddArgIdent(veh_id, caller);
		caller.AddArg(seat);		
		AddArgIdent(obj->map_id, caller);
		if (!relevant) caller.ReturnValueIgnored();
		return HandleResult<bool>(caller.Call("OnVehicleEntry", result_bool), true);
	}

	bool OnVehicleEject(const halo::s_player& player, bool forceEjected)
	{
		PhasorCaller caller;
		AddPlayerArg(&player, caller);
		if (forceEjected) caller.ReturnValueIgnored();
		return HandleResult<bool>(caller.Call("OnVehicleEject", result_bool), true);
	}

	bool OnPlayerKill(const halo::s_player& victim, const halo::s_player* killer,
		DWORD mode)
	{
		PhasorCaller caller;
		AddPlayerArg(killer, caller);
		AddPlayerArg(&victim, caller);
		caller.AddArg(mode);
		return HandleResult<bool>(caller.Call("OnPlayerKill", result_bool), true);
	}

	void OnKillMultiplier(const halo::s_player& player, DWORD multiplier)
	{
		PhasorCaller caller;
		AddPlayerArg(&player, caller);
		caller.AddArg(multiplier);
		caller.Call("OnKillMultiplier");
	}

	bool OnWeaponReload(const halo::s_player* player, halo::ident weap)
	{
		PhasorCaller caller;
		AddPlayerArg(player, caller);
		AddArgIdent(weap, caller);
		return HandleResult<bool>(caller.Call("OnWeaponReload", result_bool), true);
	}

	bool OnNameRequest(const std::string& hash, const std::string& name, 
		std::string& change_name)
	{
		using namespace Common;
		static const scripting::results_t expected = {TYPE_BOOL, TYPE_STRING};

		PhasorCaller caller;
		caller.AddArg(hash);
		caller.AddArg(name);

		Result r = caller.Call("OnNameRequest", expected);
		if (r.size() == 0) return true; // no scripts returned anything
		if (r.size() == 2) change_name = r.ReadString(1).GetValue();
		
		return r.ReadBool(0).GetValue();
	}
}}