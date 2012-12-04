#pragma once

#include <string>
#include <vector>
#include "../Common/Streams.h"

// Returns successs or failure
bool ProcessCommand(const std::string& command, COutStream& out);