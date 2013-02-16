#include "ScriptingEvents.h"
#include "Scripting.h"
#include "Phasor/Halo/Player.h"

namespace scripting { namespace events {

	static const results_t result_bool = {Common::TYPE_BOOL};
	static const results_t result_number = {Common::TYPE_NUMBER};

	/*! \todo make something so that scripts can return different types of data
	 *
	 * will be useful for OnServerChat where they can return the new string.
	 * either that or make a new function for OnServerChat which sets
	 * its return value. Also need be think about how this should interact with
	 * other scripts.
	 */
	void AddPlayerArg(const halo::s_player* player, PhasorCaller& caller)
	{
		if (player) caller.AddArg(player->memory_id);
		else caller.AddArgNil();
	}

	template <class T> T HandleResult(Result& result);

	template <> bool HandleResult<bool>(Result& result)
	{
		return !result.size() || result.ReadBool().GetValue();
	}

	bool OnTeamChange(const halo::s_player& player, bool relevant, DWORD old_team,
		DWORD new_team)
	{
		PhasorCaller caller;
		// if we're not going to process return values, let scripts know
		if (!relevant) caller.ReturnValueIgnored();
		AddPlayerArg(&player, caller);
		caller.AddArg(old_team);
		caller.AddArg(new_team);
		return HandleResult<bool>(caller.Call("OnTeamChange", result_bool));
	}

	bool OnServerCommand(const halo::s_player* player, const std::string& command)
	{
		PhasorCaller caller;
		AddPlayerArg(player, caller);
		caller.AddArg(command);
		return HandleResult<bool>(caller.Call("OnServerCommand", result_bool));
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
		return HandleResult<bool>(caller.Call("OnBanCheck", result_bool));
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
		if (r.size()) out_team = (DWORD)r.ReadNumber().GetValue();
		return r.size() != 0;
	}

	void OnPlayerSpawn(const halo::s_player& player, halo::ident m_objectId)
	{
		PhasorCaller caller;
		AddPlayerArg(&player, caller);
		caller.AddArg(m_objectId);
		caller.Call("OnPlayerSpawn");
	}

	void OnPlayerSpawnEnd(const halo::s_player& player, halo::ident m_objectId)
	{
		PhasorCaller caller;
		AddPlayerArg(&player, caller);
		caller.AddArg(m_objectId);
		caller.Call("OnPlayerSpawnEnd");
	}
}}