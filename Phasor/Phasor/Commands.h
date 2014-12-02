/* \file Commands.h
 * \brief Functions used for parsing server commands.
 */

#pragma once

#include <string>
#include <vector>
#include "Halo/HaloStreams.h"
#include "Halo/Player.h"

// stupid enum warning
#pragma warning( disable : 4482)
enum e_command_result
{
	kGiveToHalo = 0,
	kProcessed
};

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
		const std::vector<std::string>& args;
		size_t start_index, index;
		std::string function; // command being executed (ie sv_players)

		enum e_arg_types
		{
			kNone,
			kString,
			kStringOneOf,
			kInteger,
			kUnsignedInteger,
			kDouble,
			kBoolean,
			kPlayer,
			kPlayerOrHash
		};
		static const char* k_arg_names[];

		template <class T>
		T ReadNumber(e_arg_types type, T min, T max);

		bool InVector(const std::vector<std::string>& opts, 
			const std::string& to_check);		

		void RaiseError(e_arg_types expected, double min=0, double max=0,
			const std::vector<std::string>* opts = 0);
		void HasData() { if (args.size() <= index) RaiseError(kNone); }

	public:
		// args should remain for duration of this object
		explicit CArgParser(const std::vector<std::string>& args, 
			const std::string& function, size_t start_index);
		size_t size() const { return args.size() - start_index;}

		std::string ReadString(size_t min=0, size_t max=0);
		std::wstring ReadWideString(size_t min=0, size_t max=0);
		// if ignore case == true, all opts should be lowercase (not enforced)
		std::string ReadStringOneOf(const std::vector<std::string>& opts, bool ignorecase=false);
		std::wstring ReadWideStringOneOf(const std::vector<std::string>& opts, bool ignorecase=false);

		int ReadInt(int min=INT_MIN, int max=INT_MAX);
		unsigned int ReadUInt(unsigned int min=0, unsigned int max=UINT_MAX);
		double ReadDouble(double min=-DBL_MAX, double max=DBL_MAX);
		float ReadFloat(float min=-FLT_MAX, float max=FLT_MAX);
		bool ReadBool(); // accepts true/false, 1/0
		halo::s_player& ReadPlayer();
		std::string ReadPlayerHash();
		std::string ReadPlayerOrHash();
	};
}