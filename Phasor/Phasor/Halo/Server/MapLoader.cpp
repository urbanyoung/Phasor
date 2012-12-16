#include "MapLoader.h"
#include "ScriptLoader.h"
#include "../../Directory.h"
#include "../../../Common/MyString.h"
#include "../Addresses.h"
#include "Common.h"
#include "Gametypes.h"
#include "Server.h"
#include <map>

/* The reasoning behind this whole system is actually quite complex. The basic
 * idea is as follows:
 * 1.
 * Reroute the table Halo uses for storing map info to memory controlled my Phasor
 * This allows me to add new maps and have the server accept them, ie ones
 * without original names. To do this the counts need to be tracked and modified
 * too (curCount, maxCount).
 * 
 * 2.
 * All map commands are hooked, specifically sv_mapcycle_add and sv_map.
 * Phasor implements all of the logic and expands the map info struct so that
 * it has room to accommodate scripts. When a map is being loaded the scripts
 * for this entry are saved so they can be loaded later (game::OnNew).
 * Phasor needs to manage the map cycle list. When a game is being started the
 * data needs to be transferred into the memory that the server uses. 
 * 
 * 3.
 * When a map is being loaded checks need to be made. If the map being loaded
 * is to be changed (for example, map voting) the entire map data needs to be
 * rebuilt. 
 * If the map being loaded is one that isn't default (ie bloodgulch1) then
 * a bit of swapping needs to be done regarding the name of the map to load.
 * If the data is changed the code cave is notified so that the data Halo
 * processes is reloaded. 
 * The map name is later overwritten (once loaded) so it can be determined
 * exactly what map is loaded. 
 * 
 * I coded all this mid 2010, when I originally made Phasor. It
 * took awhile to get it working and I don't want to go through and rewrite
 * and re-reverse it. Unfortunately I didn't comment it very well when I made
 * it so I decided to add the information above to help you make sense of it.
*/
namespace halo { namespace server { namespace maploader
{
	// ----------------------------------------------------------------
	// Structures used for map processing
	// ----------------------------------------------------------------
	#pragma pack(push, 1)

	// Structure of map header
	struct hMapHeader
	{
		DWORD integrity; // should be 'head' if not corrupt
		DWORD type; // 7 for halo PC, ce is 0x261
		DWORD filesize;
		DWORD highSize;
		DWORD offset1;
		BYTE unk1[12]; // Unknown atm
		char name[32]; // map name
		char version[32]; // version the map was built at
		DWORD mapType; // 0 campaign, 1 multiplayer, 2 data (ui.map)
		BYTE unk2[0x798]; // first few bytes have data, rest are 00
		DWORD integrity2; // should be 'foot' if not corrupt
	};

	struct mapTableInfo
	{
		char* map;
		DWORD index;
		DWORD empty;
	};

	#pragma pack(pop)

	// ----------------------------------------------------------------
	// MAP CODE, THIS INCLUDES NON-DEFAULT MAP LOADING AND VARIOUS MAP
	// VALIDATION/INFORMATION FUNCTIONS
	// ----------------------------------------------------------------	

	// Pointers to counters used by Halo
	LPDWORD curCount = 0, maxCount = 0;

	// Map of file names (bloodgulch2 = key) and their corresponding original maps
	std::map<std::string, std::string> fileMap;

	// Stores the map which is being loaded
	char map_being_loaded_buffer[64] = {0};

	// Used for rerouting the map table
	mapTableInfo* mapTable = 0;

	// Counts used in map table
	int modCount = 0;
		
	// Stores the address of the cyclic map name, requires fixing in FixGetMap
	// 'Fixing' is just replacing the name with the actual loaded name (ie bloodgulch1)
	char* m_fixMap = 0;

	// Returns the address of the loading buffer Halo should use
	char* GetLoadingMapBuffer()
	{
		return map_being_loaded_buffer;
	}

	// This function returns the address of our map table
	DWORD GetMapTable()
	{
		return (DWORD)mapTable;
	}

	// This function checks if a map exists
	bool IsValidMap(const std::string& map)
	{
		return fileMap.find(map) != fileMap.end();
	}

	// This function reads a maps header and returns the base map (ie bloodgulch)
	bool GetMapName(const std::wstring& filePath, std::string& mapname)
	{
		CInFile file;
		if (!file.Open(filePath)) return false;

		BYTE header[2048] = {0};
		DWORD read;
		if (!file.Read(header, sizeof(header), &read)) return false;

		hMapHeader* MapHeader = (hMapHeader*)header;
		if (MapHeader->integrity == 'head' && MapHeader->type  == 7
			&& MapHeader->mapType == 1)
		{
			mapname = MapHeader->name;
		}
		return mapname.size() != 0;
	}

	// This function generates the map list
	void BuildMapList(COutStream& out)
	{
		// Allocate the memory (Halo expects it to be done w/ GlobalAlloc)
		size_t table_count = 50;
		mapTable = (mapTableInfo*)GlobalAlloc(GMEM_ZEROINIT, 
			sizeof(mapTableInfo)*table_count);

		curCount = (LPDWORD)UlongToPtr(ADDR_CURMAPCOUNT);
		maxCount = (LPDWORD)UlongToPtr(ADDR_MAXMAPCOUNT);

		std::wstring mapSearchExp;
		if (g_MapDirectory.size()) // if manually set
			mapSearchExp = g_MapDirectory + L"\\*map";
		else {
			// let halo find the map directory
			char szMapDir[1024] = {0};

			// this function returns <dir>\\.map
			typedef void (__cdecl *hResolveMapPath)(char* map, char* output);
			hResolveMapPath GetMapPath = (hResolveMapPath)(FUNC_GETMAPPATH);
			GetMapPath("", szMapDir);	

			// store the map directory
			int len = strlen(szMapDir);
			szMapDir[len-4] = 0;
			g_MapDirectory = WidenString(szMapDir);
			NDirectory::NormalizeDirectory(g_MapDirectory);

			// want to search for files of form dir\\*map
			szMapDir[len - 4] = '*';
			mapSearchExp = WidenString(szMapDir); // Phasor uses with wide paths
		}

		if (mapSearchExp.size()) 
		{
			out << L"Building map list from : " << g_MapDirectory << endl;

			std::vector<std::wstring> files;
			NDirectory::FindFiles(mapSearchExp, files);

			if (table_count < files.size()) {
				table_count = files.size() + 50;
				mapTable = (mapTableInfo*)GlobalReAlloc(mapTable,
					sizeof(mapTableInfo)*table_count, GMEM_ZEROINIT);
			}

			for (size_t x = 0; x < files.size(); x++) {
				const std::wstring& file = files[x];

				if (file.size() < 0x20) { // halo imposed limit
					std::wstring path_to_file = g_MapDirectory + file;
					std::string base_map;
					if (GetMapName(path_to_file, base_map)) {
						// name of map minus extension
						std::string map_name = NarrowString(
							file.substr(0, file.size()-4));
						ToLowercase(map_name);

						// Store the actual map and orig map names
						fileMap[map_name] = base_map;

						//out << "Added map " << map_name << " with base map " 
						//	<< base_map << endl;

						// Default map, let halo add it.
						if (base_map == map_name)
							continue;
						g_PrintStream << "Adding map " << map_name << endl;
						// Add the data into the map table
						size_t alloc_size = map_name.size() + 1;
						char* map_alloc = (char*)GlobalAlloc(GMEM_FIXED, alloc_size);
						strcpy_s(map_alloc, alloc_size, map_name.c_str());
						mapTable[*curCount].index = *curCount;
						mapTable[*curCount].map = map_alloc;
						mapTable[*curCount].empty = 1;

						// Increment counters
						*curCount += 1; *maxCount += 1;	modCount++;

					} /*else {
						out << L"Ignoring " << file << " because the base map couldn't be determined" << endl;
					}*/
				}
				else {
					out << L"The map '" << file << "' is too long and cannot be loaded." << endl;
				}
			}
		}
	}

	// Called when a map is being loaded to fix name issues
	void OnMapLoad(char* map)
	{
		// Save the map name ptr for fixing
		m_fixMap = map;

		// the name should be lowercase
		CStrToLower(map);
		strcpy_s(map_being_loaded_buffer, sizeof(map_being_loaded_buffer), map);

		// Change the passed parameter to the unmodded name to avoid loading issues
		char* baseMap = (char*)fileMap[map].c_str();
		strcpy_s(map, sizeof(map_being_loaded_buffer), baseMap);
	}

	// Called to fix the loaded map name to its actual (not base) name.
	void OnNewGame()
	{
		// Fix the map name
		strcpy_s(m_fixMap, sizeof(map_being_loaded_buffer), map_being_loaded_buffer);
	}

	// Returns the base name for a map (ie bloodgulch1 -> bloodgulch)
	bool GetBaseMapName(const char* actual_map, const char** out)
	{
		auto itr = fileMap.find(actual_map);
		if (itr == fileMap.end()) return false;
		*out = itr->second.c_str();
		return true;
	}

	// --------------------------------------------------------------------
	// Script loading
	// 
	struct s_phasor_mapcycle_entry
	{
		std::string map;
		std::wstring gametype;
		std::vector<std::string> scripts;
	};

	s_mapcycle_entry* GetMapcycleStart()
	{
		return (s_mapcycle_entry*)*(DWORD*)ADDR_MAPCYCLELIST;
	}

	s_mapcycle_entry* GetCurrentMapcycleEntry()
	{
		DWORD index = *(DWORD*)ADDR_MAPCYCLEINDEX;
		return index >= 0 && index < *(DWORD*)ADDR_MAPCYCLECOUNT
			? GetMapcycleStart() + index*sizeof(s_mapcycle_entry) : NULL;
	}

	void SetMapcycleStart(s_mapcycle_entry* new_map)
	{
		*(DWORD*)ADDR_MAPCYCLELIST = (DWORD)new_map;
	}

	DWORD GetMapcycleCount()
	{
		return *(DWORD*)ADDR_MAPCYCLECOUNT;
	}

	void SetMapcycleCount(DWORD new_count)
	{
		*(DWORD*)ADDR_MAPCYCLECOUNT = new_count;
	}

	// Cleanup the memory associated with the specified mapcycle
	void ClearMapcycle(s_mapcycle_entry* mapcycle, DWORD count) 
	{
		s_mapcycle_entry* entry = mapcycle;
		for (DWORD i = 0; i < count; i++, entry += sizeof(s_mapcycle_entry)) {
			if (entry->map) GlobalFree(entry->map);
			if (entry->gametype) GlobalFree(entry->gametype);
			if (entry->scripts) {
				for (DWORD x = 0; x < entry->scripts->count; x++)
					GlobalFree(entry->scripts->script_names[x]);
				GlobalFree(entry->scripts);
			}
		}
		GlobalFree(mapcycle);
	}

	// Clear the mapcycle Halo uses
	void ClearMapcycle()
	{
		s_mapcycle_entry* mapcycle = GetMapcycleStart();
		if (mapcycle) {
			DWORD count = GetMapcycleCount();
			ClearMapcycle(mapcycle, count);
		}
		SetMapcycleStart(NULL);
		SetMapcycleCount(0);
	}

	// Builds a new mapcycle entry into 'entry' (does memory allocation and copying)
	bool BuildNewMapcycleEntry(const std::string& map,
		const std::wstring& gametype, const std::vector<std::string>& scripts,
		s_mapcycle_entry& entry, COutStream& stream)
	{
		// Get the gametype data
		if (!gametypes::ReadGametypeData(gametype, entry.gametype_data, 
			sizeof(entry.gametype_data)))
		{ 
			stream << L"Cannot read gametype data for '" << gametype << L"'" << endl;
			return false; 
		}

		// Allocate memory for the map
		DWORD len = map.size() + 1;
		entry.map = (char*)GlobalAlloc(GMEM_FIXED, len);
		strcpy_s(entry.map, len, map.c_str());

		// Allocate memory for the gametype
		{
			std::string n_gametype = NarrowString(gametype);
			len = n_gametype.size() + 1;
			entry.gametype = (char*)GlobalAlloc(GMEM_FIXED, len);
			strcpy_s(entry.gametype, len, n_gametype.c_str());
		}

		// Allocate memory for the scripts
		entry.scripts = (s_script_list*)GlobalAlloc(GMEM_FIXED, 
			sizeof(s_script_list) 
			+ ((scripts.size() - 1) * sizeof(entry.scripts->script_names[0])));
		entry.scripts->count = scripts.size();

		// Populate the script data
		for (size_t x = 0; x < scripts.size(); x++)
		{
			entry.scripts->script_names[x] = (char*)
				GlobalAlloc(GMEM_FIXED, scripts[x].size() + 1);
			strcpy_s(entry.scripts->script_names[x], 
				scripts[x].size() + 1, scripts[x].c_str());
		}
		return true;
	}
	
	// Add maps to Halo's mapcycle. All maps, gametypes and scripts should
	// be valid.
	// Returned: The new cycle or NULL on error.
	s_mapcycle_entry* AddMapsToCycle(const std::vector<s_phasor_mapcycle_entry>& maps, 
		COutStream& stream, s_mapcycle_entry* mapcycle=GetMapcycleStart(),
		DWORD old_count = GetMapcycleCount(),
		DWORD* out_new_count = NULL)
	{
		if (mapcycle == NULL) old_count = 0;

		// Allocate the memory for the mapcycle
		s_mapcycle_entry* insert_pos = NULL;
		DWORD new_count = old_count + maps.size();
		s_mapcycle_entry* new_mapcycle = (s_mapcycle_entry*)
			GlobalAlloc(GMEM_FIXED, sizeof(s_mapcycle_entry) * new_count);
		if (old_count > 0)
			memcpy(new_mapcycle, mapcycle, old_count * sizeof(s_mapcycle_entry));
		insert_pos = new_mapcycle + (old_count * sizeof(s_mapcycle_entry));
		
		for (size_t x = 0; x < maps.size(); x++) {
			if (!BuildNewMapcycleEntry(maps[x].map, maps[x].gametype, maps[x].scripts,
				*insert_pos, stream))
			{
				ClearMapcycle(new_mapcycle, x);
				return NULL;
			}
			insert_pos += sizeof(s_mapcycle_entry);
		}
		if (out_new_count) *out_new_count = new_count;
		return new_mapcycle;
	}

	// Adds a map to the currently executing Halo mapcycle
	bool AddMapToCurrentCycle(const s_phasor_mapcycle_entry& map, 
		COutStream& stream)
	{
		std::vector<s_phasor_mapcycle_entry> entry;
		entry.push_back(map);
		DWORD new_count = 0;
		s_mapcycle_entry* new_cycle = AddMapsToCycle(entry, stream, 
			GetMapcycleStart(), GetMapcycleCount(), &new_count);
		if (new_cycle == NULL) return false;

		ClearMapcycle();
		SetMapcycleStart(new_cycle);
		SetMapcycleCount(new_count);
		return true;
	}

	// --------------------------------------------------------------------
	// Mapcycle controlling functions
	std::vector<s_phasor_mapcycle_entry> mapcycleList;
	bool in_mapcycle = false;

	e_command_result sv_mapcycle_begin(void*, 
		std::vector<std::string>& tokens, COutStream& out)
	{
		// Check if there is any cycle data
		if (!mapcycleList.size()) {
			out << L"The mapcycle is empty.";
			return e_command_result::kProcessed;
		}

		DWORD new_count = 0;
		s_mapcycle_entry* new_cycle = AddMapsToCycle(mapcycleList, out, NULL,
			0, &new_count);
		if (new_cycle == NULL) {
			out << "Unable to write the new cycle." << endl;
			return e_command_result::kProcessed;
		}

		// Activate the new mapcycle
		ClearMapcycle();
		SetMapcycleStart(new_cycle);
		SetMapcycleCount(new_count);

		// start of map cycle
		*(DWORD*)ADDR_MAPCYCLEINDEX	= -1;
		server::StartGame(mapcycleList[0].map.c_str());
		in_mapcycle = true;

		return e_command_result::kProcessed;
	}

	e_command_result sv_mapcycle_add(void*, 
		std::vector<std::string>& tokens, COutStream& out)
	{
		if (tokens.size() < 3) {
			out << L"Syntax: sv_mapcycle_add <map> <gametype> opt: <script1> <script2> ..."
				<< endl;
			return e_command_result::kProcessed;
		}
		s_phasor_mapcycle_entry entry;

		if (!IsValidMap(tokens[1]))	{
			out << tokens[1] << L" isn't a valid map." << endl;
			return e_command_result::kProcessed;
		}
		std::wstring gametype = WidenString(tokens[2]);
		if (!gametypes::IsValidGametype(gametype)) {
			out << gametype << L" isn't a valid gametype." << endl;
			return e_command_result::kProcessed;
		}

		// store scripts if there are any
		if (tokens.size() > 3) {
			for (size_t x = 3; x < tokens.size(); x++) {
				// Check the script exists
				if (scriptloader::IsValidScript(tokens[x]))
					entry.scripts.push_back(tokens[x]);
				else {
					out << tokens[x] << L" isn't a valid script."
						<< endl;
					return e_command_result::kProcessed;
				}
			}
		}

		// If we're in the mapcycle add the data to the current playlist
		if (in_mapcycle) {
			if (!AddMapToCurrentCycle(entry, out)) {
				out << L"Unable to write mapcycle data. Map addition ignored." 
					<< endl;
				return e_command_result::kProcessed;
			}
		}

		mapcycleList.push_back(entry);
		out << entry.map << " (game: " << entry.gametype << ") has been added to the mapcycle."
			<< endl;				

		return e_command_result::kProcessed;
	}

	e_command_result sv_mapcycle_del(void* exec_player, 
		std::vector<std::string>& tokens, COutStream& out)
	{
		if (tokens.size() != 2) {
			out << L"Syntax: sv_mapcycle_del <index>" << endl;
			return e_command_result::kProcessed;
		}

		DWORD index = atoi(tokens[1].c_str());
		if (index < 0 || index >= mapcycleList.size()) {
			out << L"You entered an invalid index, see sv_mapcycle." << endl;
			return e_command_result::kProcessed;
		}
		std::vector<s_phasor_mapcycle_entry> newMapCycle = mapcycleList;
		newMapCycle.erase(mapcycleList.begin() + index);

		DWORD new_count = 0;
		s_mapcycle_entry* new_cycle = AddMapsToCycle(newMapCycle, out, NULL,
			0, &new_count);
		if (new_cycle == NULL) {
			out << "Unable to create the new mapcycle. Deletion ignored." << endl;
			return e_command_result::kProcessed;
		}
		mapcycleList = newMapCycle;

		// Make sure the index Halo's using is still valid
		DWORD cur = *(DWORD*)ADDR_MAPCYCLEINDEX;
		if (cur	>= mapcycleList.size())
			*(DWORD*)ADDR_MAPCYCLEINDEX = -1; // restart the cycle
		else if (index <= cur && cur > -1)
			*(DWORD*)ADDR_MAPCYCLEINDEX = cur - 1;

		// Display cycle as it is now
		//sv_mapcycle(exec_player, tokens, out);
		return e_command_result::kProcessed;
	}

	/*bool sv_mapcycle(void*, 
		std::vector<std::string>& tokens, COutStream& out)
	{
		halo::hprintf("   Map                  Variant         Script(s)");

		std::vector<s_phasor_mapcycle_entry*>::iterator itr = mapcycleList.begin();

		int x = 0;
		while (itr != mapcycleList.end())
		{
			std::string szEntry =  m_sprintf_s("%i  %s", x, (*itr)->map.c_str());

			int tabCount = 3 - (szEntry.size() / 8);

			for (int i = 0; i < tabCount; i++)
				szEntry += "\t";

			szEntry += (*itr)->gametype;
			int len = (*itr)->gametype.size();
			tabCount = 2 - (len / 8);

			for (int i = 0; i < tabCount; i++)
				szEntry += "\t";

			// Loop through and add scripts (if any)
			for (size_t i = 0; i < (*itr)->scripts.size(); i++)
			{
				if (i != 0)
					szEntry += ",";
				szEntry += (*itr)->scripts[i];
			}

			if (!(*itr)->scripts.size())
				szEntry += "<no scripts>";

			std::string output = ExpandTabsToSpace(szEntry.c_str());
			halo::hprintf("%s", output.c_str());

			itr++; x++;
		}

		return true;
	}

	bool sv_map(halo::h_player* exec, std::vector<std::string>& tokens)
	{
		if (tokens.size() >= 3)
		{
			std::vector<std::string> scripts;

			if (tokens.size() > 3)
			{
				for (size_t x = 3; x < tokens.size(); x++)
				{
					// Check the script exists
					if (Lua::CheckScriptVailidity(tokens[x]))
						scripts.push_back(tokens[x]);
					else
					{
						logging::LogData(LOGGING_PHASOR, "sv_map: %s isn't a valid script and therefore can't be used.", tokens[x].c_str());
						halo::hprintf("sv_map: %s isn't a valid script and therefore can't be used.", tokens[x].c_str());
					}
				}
			}

			// Clear all currently loaded maps
			ClearCurrentMapData();

			// Load this map
			if (LoadCurrentMap(tokens[1].c_str(), tokens[2].c_str(), scripts))
			{
				if (vote::IsEnabled())
				{
					halo::hprintf("Map voting has been disabled.");
					vote::SetMapVote(false);
				}

				*(DWORD*)ADDR_MAPCYCLEINDEX	= -1;

				// Start the game
				StartGame(tokens[1].c_str());

				// Stop map voting from calling sv_mapcycle_begin if its set to
				//mapvote::DisableMapcycle();

				mapcycle = false;

				halo::hprintf("%s is being loaded with game type %s", tokens[1].c_str(), tokens[2].c_str());
			}			
		}
		else
			halo::hprintf("Syntax: sv_map <map> <gametype> opt: <script1> <script2> ...");

		return true;
	}

	bool sv_end_game(halo::h_player* exec, std::vector<std::string>& tokens)
	{
		mapcycle = false;
		return false;
	}

	// Custom functions
	bool sv_reloadscripts(halo::h_player* exec, std::vector<std::string>& tokens)
	{
		Lua::ReloadScripts();
		halo::hprintf("All currently loaded scripts have been reloaded.");

		return true;
	}*/
}}}