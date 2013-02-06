#pragma once

#include "../Manager.h"

#define PHASOR_API_ARGS Manager::CallHandler& handler, \
	Common::Object::unique_deque& args, \
	Common::Object::unique_list& results

namespace PhasorAPI
{
	void Register(Manager::ScriptState& state, bool deprecated);
}