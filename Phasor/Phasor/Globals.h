#pragma once

#include "Logging.h"
#include "GameLogging.h"
#include "PhasorThread.h"
#include "Halo/Server/ServerStreams.h"
#include "../Common/Timers.h"

// All of these are defined in main.cpp
extern PhasorThread g_Thread;
extern Timers g_Timers;
extern std::unique_ptr<CGameLog> g_GameLog;
extern std::unique_ptr<CScriptsLog> g_ScriptsLog;
extern std::unique_ptr<CPhasorLog> g_PhasorLog;
extern std::unique_ptr<CRconLog> g_RconLog;
extern halo::server::CHaloPrintStream g_PrintStream;