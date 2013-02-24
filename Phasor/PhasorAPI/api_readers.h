#pragma once

#include <vector>
#include <string>
#include "../Phasor/Halo/Game/Game.h"
#include "../Phasor/Halo/Server/Server.h"
#include "PhasorAPI.h"

// Raises an invalid player error
void RaisePlayerError(Manager::CallHandler& handler, int player_id);

// Reads the message argument and parses it, replacing {x} with player x's 
// name and splitting the message into a new one at each \n
std::vector<std::wstring> ReadString(Common::Object& obj);

// Don't apply any processing like in ReadString
std::string ReadRawString(Common::Object& obj);

// Read the player, if strict is true an error is raised if the player doesn't
// exist.
halo::s_player* ReadPlayer(Manager::CallHandler& handler, Common::Object& playerObj, bool strict);

// Read number of specified type.
template <typename T> T ReadNumber(const Common::Object& obj)
{
	const Common::ObjNumber* num = (const Common::ObjNumber*)&obj;
	return (T)num->GetValue();
}

// Reads a boolean
bool ReadBoolean(const Common::Object& obj);


void AddResultNil(Common::Object::unique_list& results);
void AddResultString(const std::string& str, Common::Object::unique_list& results);
void AddResultString(const std::wstring& str, Common::Object::unique_list& results);
void AddResultNumber(double value, Common::Object::unique_list& results);
void AddResultBool(bool b, Common::Object::unique_list& results);
void AddResultPtr(void* ptr, Common::Object::unique_list& result);
void AddResultTable(const std::vector<std::string>& data, Common::Object::unique_list& result);