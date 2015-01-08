#pragma once

#include "../../../Common/Types.h"
#include "../../../Common/Streams.h"
#include "../Addresses.h"
#include "../../Commands.h"
#include <vector>

enum e_command_result;

namespace halo { namespace server { namespace maploader 
{
	struct s_phasor_mapcycle_entry
	{
		std::string map;
		std::wstring gametype;
		std::vector<std::string> scripts;
	};

#pragma pack(push, 1)
	struct s_script_list
	{
		DWORD count;
		char** script_names;
	};
	struct s_mapcycle_entry
	{
		char* map;
		char* gametype;
		s_script_list* scripts;
		BYTE gametype_data[GAMET_BUFFER_SIZE];
	};
	static_assert(sizeof(s_mapcycle_entry) == CONST_MENTRY_SIZE, 
		"sizeof(s_mapcycle_entry) != CONST_MENTRY_SIZE");

	struct s_mapcycle_header
	{
		s_mapcycle_entry* games;
		DWORD cur_count;
		DWORD allocated_count;
		DWORD current; // index of game being executed
	};
#pragma pack(pop)

	void Initialize(COutStream& out);

	// Checks if the map, gametype and all scripts are valid.
	bool ValidateUserInput(const s_phasor_mapcycle_entry& entry, COutStream& out);

	// Effectively executes sv_map to run a new game
	bool LoadGame(const s_phasor_mapcycle_entry& game, COutStream& out);

	bool ReplaceHaloMapEntry(s_mapcycle_entry* old, 
		const s_phasor_mapcycle_entry& new_entry,
		COutStream& out);

	// Checks if a map exists
	bool IsValidMap(const std::string& map);

	//Non-default map loading
	// --------------------------------------------------------------------
#ifdef PHASOR_PC
	// Returns the address of the loading buffer Halo should use
	char* GetLoadingMapBuffer();

	// Generates the map list
	//! \todo fix for ce
	void BuildMapList(COutStream& out);

	// This function returns the address of our map table
	DWORD GetMapTable();

	// Called when a map is being loaded.
	void OnMapLoad(char* map);

	// Called to fix the loaded map name
	void OnNewGame();

	// Returns the base name for a map (ie bloodgulch1 -> bloodgulch)
	bool GetBaseMapName(const char* actual_map, const char** out);
#endif	

	// Get a pointer to the current map in the playlist
	s_mapcycle_entry* GetCurrentMapcycleEntry();

	e_command_result sv_mapcycle_begin(void*, 
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_mapcycle_add(void*, 
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_mapcycle_del(void* exec_player, 
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_mapcycle(void*, 
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_map(void*, 
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_end_game(void*,
		commands::CArgParser& args, COutStream&);
    e_command_result sv_refresh_maps(void*, commands::CArgParser&, COutStream& out);
}}}