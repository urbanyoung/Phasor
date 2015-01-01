#pragma once

#include "Logging.h"
#include "GameLogging.h"
#include "PhasorThread.h"
#include "Halo/HaloStreams.h"
#include "../Common/Timers.h"

// defined in main.cpp
extern PhasorThread g_Thread;
extern Timers g_Timers; 
extern LARGE_INTEGER g_FrequencyMs;
extern std::unique_ptr<CGameLog> g_GameLog;
extern std::unique_ptr<CScriptsLog> g_ScriptsLog;
extern std::unique_ptr<CPhasorLog> g_PhasorLog;
extern std::unique_ptr<CRconLog> g_RconLog;
extern std::unique_ptr<halo::CHaloPrintStream> g_PrintStream;
