#pragma once

#include "../../Globals.h"
#include "../Server/Server.h"

using namespace halo;

void __stdcall dUpdateAllObjects();
void __stdcall dUpdateObject(ident);

//Player_Lead has priority over default_lead
extern short default_lead; // -1 means use default lead, -2 means turn leadcontrol off
extern short Player_Lead[16]; // -1 means use default lead, -2 means turn leadcontrol off
extern bool useAveragePing;