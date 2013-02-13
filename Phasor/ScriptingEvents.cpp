#include "ScriptingEvents.h"
#include "Scripting.h"
#include "Phasor/Halo/Player.h"

namespace scripting { namespace events {

	static const results_t result_bool = {Common::TYPE_BOOL};

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

	bool OnTeamChange(const halo::s_player& player, bool relevant, DWORD old_team)
	{
		PhasorCaller caller;
		// if we're not going to process return values, let scripts know
		if (!relevant) caller.ReturnValueIgnored();
		AddPlayerArg(&player, caller);
		caller.AddArg(old_team);
		caller.AddArg(player.mem->team);
		return HandleResult<bool>(caller.Call("OnTeamChange", result_bool));
	}

	bool OnServerCommand(const halo::s_player* player, const std::string& command)
	{
		PhasorCaller caller;
		AddPlayerArg(player, caller);
		caller.AddArg(command);
		return HandleResult<bool>(caller.Call("OnServerCommand", result_bool));
	}

}}