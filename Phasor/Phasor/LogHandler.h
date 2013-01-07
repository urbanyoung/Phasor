#pragma once

#include "Commands.h"

namespace logging
{
	using namespace commands;
	e_command_result sv_logname(void*, CArgParser& args, COutStream& out);

}