#pragma once

#include "Scripting.h"

namespace halo {
	struct ident;
	struct s_player;
}

namespace scripting {

	static const results_t result_bool = {Common::TYPE_BOOL};
	static const results_t result_number = {Common::TYPE_NUMBER};

	/*! \todo make something so that scripts can return different types of data
	 *
	 * will be useful for OnServerChat where they can return the new string.
	 * either that or make a new function for OnServerChat which sets
	 * its return value. Also need be think about how this should interact with
	 * other scripts.
	 */
	void AddPlayerArg(const halo::s_player* player, PhasorCaller& caller);
	void AddArgIdent(const halo::ident id, PhasorCaller& caller);

	template <class T> T HandleResult(Result& result, const T& default_value);
	
	enum e_ident_bool_empty {
		kIdentSet = 0,
		kBoolSet,
		kEmptySet
	};
	
	e_ident_bool_empty HandleResultIdentOrBool(Result& result, halo::ident& id, bool& b);
}