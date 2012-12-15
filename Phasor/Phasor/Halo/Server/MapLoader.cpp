#include "MapLoader.h"
#include "../../Directory.h"
#include "../../../Common/MyString.h"
#include "../Addresses.h"
#include "Common.h"
#include "Gametypes.h"
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

	void ClearMapcycle()
	{
		s_mapcycle_entry* mapcycle = GetMapcycleStart();
		if (mapcycle) {
			DWORD count = GetMapcycleCount();
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
		SetMapcycleStart(NULL);
		SetMapcycleCount(0);
	}

	// Loads a map into the currently loaded cycle
	bool LoadCurrentMap(const std::string& map, const std::wstring& gametype,
		std::vector<std::string>& scripts, COutStream& stream)
	{
		if (!IsValidMap(map.c_str()))	{
			stream << L"'" << map << L"' isn't a valid map." << endl;
			return false; 
		}

		if (gametypes::IsValidGametype(gametype)) {
			stream << L"'" << gametype << L"' isn't a valid gametype." << endl;
			return false; 
		}

		s_mapcycle_entry entry;

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

		// now add the new entry to the current cycle
		s_mapcycle_entry* new_entry_pos = GetMapcycleStart();
		if (new_entry_pos) { // already map cycle, need to expand
			DWORD count = GetMapcycleCount();
			s_mapcycle_entry* new_mapcycle = (s_mapcycle_entry*)
				GlobalAlloc(GMEM_FIXED, sizeof(s_mapcycle_entry) * (count + 1));
			memcpy(new_mapcycle, new_entry_pos, count * sizeof(s_mapcycle_entry));
			new_entry_pos = new_mapcycle + (count * sizeof(s_mapcycle_entry));
		} else { // no allocation
			new_entry_pos = (s_mapcycle_entry*)GlobalAlloc(GMEM_FIXED, 
				sizeof(s_mapcycle_entry));
			SetMapcycleStart(new_entry_pos);
			SetMapcycleCount(1);
		}

		memcpy(new_entry_pos, &entry, sizeof(entry));
		return true;
	}
}}}