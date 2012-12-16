#pragma once

#include <string>
#include <vector>
#include "../Common/Streams.h"

enum e_command_result; // Server/Server.h

namespace commands 
{
	e_command_result ProcessCommand(const std::string& command, COutStream& out,
		void* exec_player=NULL);
}