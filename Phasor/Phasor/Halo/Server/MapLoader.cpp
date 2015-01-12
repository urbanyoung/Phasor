#include "MapLoader.h"
#include "ScriptLoader.h"
#include "../../Directory.h"
#include "../../../Common/MyString.h"
#include "../../../Common/FileIO.h"
#include "../Addresses.h"
#include "../../Globals.h"
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

	static const size_t kMaxMapLength = 0x20;

#ifdef PHASOR_PC
	// ----------------------------------------------------------------
	// MAP CODE, THIS INCLUDES NON-DEFAULT MAP LOADING AND VARIOUS MAP
	// VALIDATION/INFORMATION FUNCTIONS
	// ----------------------------------------------------------------	

	// Pointers to counters used by Halo
	LPDWORD curCount = 0, maxCount = 0;

	// Map of file names (bloodgulch2 = key) and their corresponding original maps
	std::map<std::string, std::string> fileMap;

	// Stores the map which is being loaded
	char map_being_loaded_buffer[kMaxMapLength] = {0};

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

    void FreeMapList() {
        for (auto x = 0; x < *curCount; x++) {
            GlobalFree(mapTable[x].map);
        }
        GlobalFree(mapTable);
        *curCount = 0;
        *maxCount = 0;
        modCount = 0;
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

			std::list<std::wstring> files;
			NDirectory::FindFiles(mapSearchExp, files);

			if (table_count < files.size()) {
				table_count = files.size() + 50;
				mapTable = (mapTableInfo*)GlobalReAlloc(mapTable,
					sizeof(mapTableInfo)*table_count, GMEM_ZEROINIT);
			}

			for (auto itr = files.cbegin(); itr != files.cend(); ++itr) {
				const std::wstring& file = *itr;

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
						*g_PrintStream << "Adding map " << map_name << endl;
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
#endif

#ifdef PHASOR_CE

	// Checks if a map exists
	bool IsValidMap(const std::string& map)
	{
		const char* map_cstr = map.c_str();
		bool valid = false;
		__asm
		{
			pushad

			mov eax, map_cstr
			call dword ptr ds:[FUNC_VERIFYMAP_CE]
			cmp eax, -1
			je MAP_NOT_FOUND
			mov valid, 1
MAP_NOT_FOUND:
			popad

		}
		return valid;
	}
	
#endif

	// This function is effectively sv_map_next
	void StartGame(const char* map)
	{
		if (*(DWORD*)ADDR_GAMEREADY != 2) {
			// start the server not just game
			__asm
			{
				pushad
				MOV EDI,map
				CALL dword ptr ds:[FUNC_PREPAREGAME_ONE]
				push 0
					push esi // we need a register for a bit
				mov esi, dword ptr DS:[ADDR_PREPAREGAME_FLAG]
				mov byte PTR ds:[esi], 1
					pop esi

				call dword ptr ds:[FUNC_PREPAREGAME_TWO]
				add esp, 4
				popad
			}
		}
		else {
			// Halo 1.09 addresses
			// 00517845  |.  BF 90446900   MOV EDI,haloded.00694490                                  ;  UNICODE "ctf1"
			//0051784A  |.  F3:A5         REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS:[ESI]
			//0051784C  |.  6A 00         PUSH 0
			//	0051784E  |.  C605 28456900>MOV BYTE PTR DS:[694528],1
			//	00517855  |.  E8 961B0000   CALL haloded.005193F0                                     ;  start server
			__asm
			{
				pushad
					call dword ptr ds:[FUNC_EXECUTEGAME]
				popad
			}
		}
	}

	// --------------------------------------------------------------------
	// Script loading
	// 
	
	s_mapcycle_header* g_mapcycle_header = NULL;

	class CHaloMapcycle
	{
	private:
		s_mapcycle_entry* start;
		size_t cur_count, allocated_count;
		bool active;

		// called when this cycle is no longer in use by halo
		void NoLongerActive() {	active = false;	}

		// called when this cycle is in use by halo
		void Active() { active = true; }

		// free the current cycle
		void Free()
		{
			Free(start, cur_count);
			GlobalFree(start);
			start = NULL;
			if (active) {
				g_mapcycle_header->games = NULL;
				g_mapcycle_header->cur_count = 0;
				g_mapcycle_header->allocated_count = 0;
				g_mapcycle_header->cur_count = -1;
			}
		}

		// Expand the current allocation by n items
		bool Expand(size_t n)
		{
			size_t new_count = allocated_count + n;
			s_mapcycle_entry* new_cycle = (s_mapcycle_entry*)
				GlobalAlloc(GMEM_FIXED, sizeof(s_mapcycle_entry) * new_count);
			if (!new_cycle) return false;
			memcpy(new_cycle, start, sizeof(s_mapcycle_entry) * cur_count);
			GlobalFree(start); // just the containing block, not each entry
			start = new_cycle;
			allocated_count = new_count;
			if (active) {
				g_mapcycle_header->games = new_cycle;
				g_mapcycle_header->allocated_count = allocated_count;
			}
			return true;
		}

	public:
		CHaloMapcycle(size_t allocated_count = 3)
			: allocated_count(allocated_count), cur_count(0), active(false)
		{
			start = (s_mapcycle_entry*)
				GlobalAlloc(GMEM_FIXED, sizeof(s_mapcycle_entry) * allocated_count);
		}
		~CHaloMapcycle()
		{
			Free();
		}

		// Free count items from the cycle pointer to by free_start
		// free_start doesn't need to be the start of the cycle
		static void Free(s_mapcycle_entry* free_start, DWORD count)
		{
			s_mapcycle_entry* entry = free_start;
			for (DWORD i = 0; i < count; i++, entry++) {
				if (entry->map)	GlobalFree(entry->map);
				if (entry->gametype) GlobalFree(entry->gametype);
				if (entry->scripts) {
					for (DWORD x = 0; x < entry->scripts->count; x++)
						GlobalFree(entry->scripts->script_names[x]);
					GlobalFree(entry->scripts);
				}
			}
		}

		static bool BuildNewEntry(const std::string& map,
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
			entry.map = (char*)GlobalAlloc(GMEM_FIXED, kMaxMapLength);
			strcpy_s(entry.map, kMaxMapLength, map.c_str());

			// Allocate memory for the gametype
			{
				std::string n_gametype = NarrowString(gametype);
				DWORD len = n_gametype.size() + 1;
				entry.gametype = (char*)GlobalAlloc(GMEM_FIXED, len);
				strcpy_s(entry.gametype, len, n_gametype.c_str());
			}

			if (scripts.size() > 0) {
				DWORD size = sizeof(s_script_list);
				// Allocate memory for the scripts
				entry.scripts = (s_script_list*)GlobalAlloc(GMEM_FIXED, 
					size);
				entry.scripts->count = scripts.size();
				entry.scripts->script_names = (char**)GlobalAlloc(GMEM_FIXED,
					sizeof(entry.scripts->script_names[0])*scripts.size());

				// Populate the script data
				for (size_t x = 0; x < scripts.size(); x++)
				{
					entry.scripts->script_names[x] = (char*)
						GlobalAlloc(GMEM_FIXED, scripts[x].size() + 1);
					strcpy_s(entry.scripts->script_names[x], 
						scripts[x].size() + 1, scripts[x].c_str());
				}
			} else entry.scripts = NULL;

			return true;
		}

		// load the specified game into this cycle, expanding if necessary.
		bool AddGame(const s_phasor_mapcycle_entry& game, COutStream& out)
		{
			if (cur_count >= allocated_count) {
				if (!Expand(3)) return false;
			}
			bool success = BuildNewEntry(game.map, game.gametype, game.scripts,
				*(start + cur_count), out);
			if (!success) return false;
			cur_count++;
			if (active)	{
				g_mapcycle_header->cur_count = cur_count;
			}
			return true;
		}

		// Add a list of games to the mapcycle
		bool AddGames(std::vector<s_phasor_mapcycle_entry>& games, COutStream& out)
		{
			size_t old_size = cur_count;
			if (!Reserve(games.size())) return false;
			for (size_t x = 0; x < games.size(); x++) {
				if (!AddGame(games[x], out)) { //todo: test
					// on error roll back all changes
					Free(start + old_size, x);
					cur_count = old_size;
					return false;
				}
			}
			return true;
		}

		// Remove a game from the current cycle
		bool DeleteGame(size_t index)
		{
			if (index < 0 || index >= cur_count) return false;
			s_mapcycle_entry* to_remove = start + index;
			Free(to_remove, 1);
			DWORD copy_count = cur_count - index - 1;
			memcpy(to_remove, to_remove + 1, copy_count * sizeof(to_remove[0]));
			cur_count--;

			if (active) {
				g_mapcycle_header->cur_count = cur_count;

				// Make sure the index Halo's using is still valid
				if (g_mapcycle_header->current >= cur_count)
					g_mapcycle_header->current = -1; // restart cycle
				else if (g_mapcycle_header->current == index)
					g_mapcycle_header->current--;
			}

			return true;
		}

		// Return number of entries in the cycle
		size_t size() { return cur_count; }

		// Ensures there is space for at least n ADDITIONAL items
		bool Reserve(int n)
		{
			int free_count = (int)allocated_count - cur_count;
			if (free_count >= n) return true;
			size_t new_size = allocated_count + n - free_count;
			return Expand(new_size - allocated_count);
		}

		friend class CHaloMapcycleLoader;
	};

	// Used for changing which mapcycle halo is currently using
	class CHaloMapcycleLoader
	{
	private:
		std::unique_ptr<CHaloMapcycle> active;
		s_mapcycle_header& mapcycle_header;

	public:
		CHaloMapcycleLoader(s_mapcycle_header& mapcycle_header)
			: mapcycle_header(mapcycle_header)
		{
		}

		// Changes the active mapcycle and restarts the game
		void SetActiveCycle(std::unique_ptr<CHaloMapcycle> new_cycle)
		{
			if (new_cycle->size() == 0) return;
			if (active) active->NoLongerActive();
			active = std::move(new_cycle);
			active->Active();
			mapcycle_header.games = active->start;
			mapcycle_header.cur_count = active->cur_count;
			mapcycle_header.allocated_count = active->allocated_count;
			mapcycle_header.current = -1;
			StartGame(active->start->map);
		}

		bool is_empty() { return !active; }
		CHaloMapcycle& GetActive() { return *active; }

		friend class CHaloMapcycle;
	};

	std::unique_ptr<CHaloMapcycleLoader> cycle_loader;
	std::vector<s_phasor_mapcycle_entry> mapcycleList;
	bool in_mapcycle = false;

	void InitializeMapcycle()
	{
		g_mapcycle_header = (s_mapcycle_header*)ADDR_MAPCYCLELIST;
		cycle_loader = std::unique_ptr<CHaloMapcycleLoader>(new CHaloMapcycleLoader(*g_mapcycle_header));
	}

	s_mapcycle_entry* GetCurrentMapcycleEntry()
	{
		// will break (give wrong data) if current map removed from cycle.
		// only used in one place so don't really care
		DWORD index = g_mapcycle_header->current;
		return index >= 0 && index < g_mapcycle_header->cur_count
			? g_mapcycle_header->games + index : NULL;
	}
	
	// Checks if the map, gametype and all scripts are valid.
	bool ValidateUserInput(const s_phasor_mapcycle_entry& entry, COutStream& out)
	{
		if (!IsValidMap(entry.map))	{
			out << entry.map << L" isn't a valid map." << endl;
			return false;
		}

		if (!gametypes::IsValidGametype(entry.gametype)) {
			out << entry.gametype << L" isn't a valid gametype." << endl;
			return false;
		}

		bool success = true;
		for (size_t x = 0; x < entry.scripts.size(); x++) {
			if (!scriptloader::IsValidScript(entry.scripts[x])) {
				out << entry.scripts[x] << L" isn't a valid script." << endl;
				success = false;
			}
		}
		return success;
	}

	// Effectively executes sv_map to run a new game
	bool LoadGame(const s_phasor_mapcycle_entry& game, COutStream& out)
	{
		std::unique_ptr<CHaloMapcycle> new_cycle = 
			std::unique_ptr<CHaloMapcycle>(new CHaloMapcycle());
		
		if (!new_cycle->AddGame(game, out)) {
			out << L"Previous errors prevent the game from being started." << endl;
			return false;
		}		

		// start the new game
		cycle_loader->SetActiveCycle(std::move(new_cycle));
		in_mapcycle = false;
		return true;
	}

	bool ReplaceHaloMapEntry(s_mapcycle_entry* old, 
		const s_phasor_mapcycle_entry& new_entry,
		COutStream& out)
	{
		s_mapcycle_entry tmp;
		
		if (!CHaloMapcycle::BuildNewEntry(new_entry.map,
			new_entry.gametype, new_entry.scripts, tmp, out))
		{
			return false;
		}
		CHaloMapcycle::Free(old, 1);
		*old = tmp;	
		return true;
	}

	bool ReadGameDataFromUser(s_phasor_mapcycle_entry& entry,
		commands::CArgParser& args, COutStream& out)
	{
		entry.map = args.ReadString();
		entry.gametype = args.ReadWideString();
		for (size_t x = 2; x < args.size(); x++)
			entry.scripts.push_back(args.ReadString());
		return ValidateUserInput(entry, out);
	}

	e_command_result sv_mapcycle_begin(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		// Check if there is any cycle data
		if (!mapcycleList.size()) {
			out << L"The mapcycle is empty." << endl;
			return e_command_result::kProcessed;
		}
		std::unique_ptr<CHaloMapcycle> new_cycle = 
			std::unique_ptr<CHaloMapcycle>(new CHaloMapcycle());
		if (!new_cycle) {
			out << L"Cannot allocate memory for new mapcycle." << endl;
			return e_command_result::kProcessed;
		}

		if (!new_cycle->AddGames(mapcycleList, out)) {
			out << L"Previous errors prevent the mapcycle from being started." << endl;
			return e_command_result::kProcessed;
		}

		cycle_loader->SetActiveCycle(std::move(new_cycle));
		in_mapcycle = true;

		return e_command_result::kProcessed;
	}

	e_command_result sv_mapcycle_add(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		s_phasor_mapcycle_entry entry;
		if (!ReadGameDataFromUser(entry, args, out)) return e_command_result::kProcessed;

		// If we're in the mapcycle add the data to the current playlist
		if (in_mapcycle) {
			if (!cycle_loader->GetActive().AddGame(entry, out)) {
				out << L"Cannot write addition to current cycle. Addition ignored."
					<< endl;
				return e_command_result::kProcessed;
			}
		}

		mapcycleList.push_back(entry);
		out << entry.map << " (game: '" << entry.gametype << "') has been added to the mapcycle."
			<< endl;				

		return e_command_result::kProcessed;
	}

	e_command_result sv_mapcycle_del(void* exec_player, 
		commands::CArgParser& args, COutStream& out)
	{
		unsigned int index = args.ReadUInt();
		if (index < 0 || index >= mapcycleList.size()) {
			out << L"You entered an invalid index, see sv_mapcycle." << endl;
			return e_command_result::kProcessed;
		}

		mapcycleList.erase(mapcycleList.begin() + index);
		if (in_mapcycle)
			cycle_loader->GetActive().DeleteGame(index);

		// Display cycle as it is now
		return sv_mapcycle(exec_player, args, out);
	}

	e_command_result sv_mapcycle(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		out.wprint(L"   %-20s%-20s%s", L"Map", L"Variant", L"Script(s)");
		const wchar_t* fmt = L"%-3i%-20s%-20s%s";

		for (size_t x = 0; x < mapcycleList.size(); x++)
		{
			s_phasor_mapcycle_entry& entry = mapcycleList[x];
			std::string scripts_desc;
			for (size_t i = 0; i < entry.scripts.size(); i++) {
				if (i != 0) scripts_desc += ",";
				scripts_desc += entry.scripts[i].c_str();
			}			
			std::wstring scripts_desc_w = WidenString(scripts_desc);
			if (!scripts_desc_w.size()) scripts_desc_w = L"<no scripts>";
			std::wstring map_w = WidenString(entry.map);
			out.wprint(fmt, x, map_w.c_str(), entry.gametype.c_str(), 
				scripts_desc_w.c_str());
		}

		return e_command_result::kProcessed;
	}

	e_command_result sv_map(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		s_phasor_mapcycle_entry entry;
		if (!ReadGameDataFromUser(entry, args, out)) return e_command_result::kProcessed;
	
		LoadGame(entry, out);
		return e_command_result::kProcessed;
	}

	e_command_result sv_end_game(void*, commands::CArgParser&, COutStream&)
	{
		in_mapcycle = false;
		return e_command_result::kGiveToHalo;
	}

   /* e_command_result sv_refresh_maps(void*, commands::CArgParser&, COutStream& out)
    {
    won't work for ce, also need to have opption to load all maps (by default I just do non-default ones)
        BuildMapList(out);
        DWORD table = server::maploader::GetMapTable();
        WriteBytes(PATCH_MAPTABLE, &table, 4);
        return e_command_result::kProcessed;
    }*/

	// --------------------------------------------------------------------
	// Initialize the system
	void Initialize(COutStream& out)
	{
#ifdef PHASOR_PC
		BuildMapList(out);
#endif
		InitializeMapcycle();

	}
}}}