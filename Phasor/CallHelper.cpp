#include "CallHelper.h"
#include "Phasor/Halo/Player.h"
#include "Phasor/Halo/Halo.h"

namespace scripting {
	void AddPlayerArg(const halo::s_player* player, PhasorCaller& caller)
	{
		if (player) caller.AddArg(player->memory_id);
		else caller.AddArgNil();
	}

	void AddArgIdent(const halo::ident id, PhasorCaller& caller)
	{
		if (!id.valid()) caller.AddArgNil();
		else caller.AddArg(id);
	}

	template <> bool HandleResult<bool>(Result& result, const bool& default_value)
	{
		return result.size() ? result.ReadBool().GetValue() : default_value;
	}
}