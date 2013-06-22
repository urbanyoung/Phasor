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
		return result.size() ? result.ReadBool(0).GetValue() : default_value;
	}

	e_ident_bool_empty HandleResultIdentOrBool(Result& result, halo::ident& id, bool& b)
	{
		if (!result.size()) return kEmptySet;
		DWORD value = (DWORD)result.ReadNumber(0).GetValue();
		if (value == 1 || value == 0) {
			b = value == 1;
			return kBoolSet;
		} else {
			id = halo::make_ident(value);
			return kIdentSet;
		}
	}
}