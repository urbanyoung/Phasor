#pragma once

#include <string>
#include <vector>
#include "../Common/Streams.h"
#include "Halo/Player.h"

enum e_command_result; // Server/Server.h

namespace commands 
{
	e_command_result ProcessCommand(const std::string& command, 
		COutStream& out, halo::s_player* exec_player=NULL);

// Used for parsing user input in server commands.
// If an error occurs the Read functions throw and exception which
// is caught in ProcessCommand.
class CArgParser
{
private:
	std::vector<std::string> args;
	size_t start_index, index;
	std::string function; // command being executed (ie sv_players)

	enum e_arg_types
	{
		kNone,
		kString,
		kInteger,
		kDouble,
		kPlayer
	};
	static const char* k_arg_names[];

	template <class T>
	T StringToNumber(const char* start, char** end);
	template <>
	int StringToNumber<int>(const char* start, char** end) { return strtol(start, end, 10);	}
	template <>
	unsigned int StringToNumber<unsigned int>(const char* start, char** end) { return strtoul(start, end, 10); }
	template <>
	double StringToNumber<double>(const char* start, char** end) { return strtod(start, end);}
	
	template <class T>
	T ReadNumber(e_arg_types type)
	{
		HasData();
		std::string& arg = args[index++];
		const char* start = arg.c_str(), *expected_end = start + arg.size();
		char* end;
		T value = StringToNumber<T>(start, &end);
		if (end != expected_end) RaiseError(type);
		return value;
	}

	void RaiseError(e_arg_types expected, int size=-1);
	void HasData() { if (args.size() <= index) RaiseError(kNone); }

public:
	explicit CArgParser(const std::vector<std::string>& args, 
		const std::string& function, size_t start_index);
	size_t size() const { return args.size() - start_index;}

	std::string ReadString(size_t len = -1);
	std::wstring ReadWideString(size_t len = -1);
	int ReadInt();
	unsigned int ReadUInt();
	double ReadDouble();
	float ReadFloat();
	halo::s_player& ReadPlayer();
	const std::string& ReadPlayerHash();
};
}