#ifndef NOLEAD_H
#define NOLEAD_H

#pragma once

#include "../Addresses.h"
#include "../../Globals.h"
#include "../Server/Server.h"

void __stdcall dUpdateAllObjects();
void __stdcall dUpdateObject(struct ident);

extern short default_lead;
extern short Player_Lead[16];
#endif