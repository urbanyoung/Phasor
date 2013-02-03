#pragma once

#include "Player.h"
#include "../Commands.h"

namespace halo { namespace alias 
{
	void Initialize();
	void OnPlayerJoin(s_player& player);

	using namespace commands;
	e_command_result sv_alias_search(void*, CArgParser& args, CCheckedStream& out);
	e_command_result sv_alias_hash(void*, CArgParser& args, CCheckedStream& out);
	e_command_result sv_alias_enable(void*, CArgParser& args, CCheckedStream& out);

}}