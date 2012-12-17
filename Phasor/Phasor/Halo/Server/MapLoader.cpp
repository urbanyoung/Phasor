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
#endif
	// --------------------------------------------------------------------
	// Script loading
	// 
	struct s_phasor_mapcycle_entry
	{
		std::string map;
		std::wstring gametype;
		std::vector<std::string> scripts;
	};
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

		// Free count items from the cycle pointer to by free_start
		// free_start doesn't need to be the start of the cycle
		void Free(s_mapcycle_entry* free_start, DWORD count)
		{
			g_PrintStream.print("Clearing with count %i", cur_count);
			s_mapcycle_entry* entry = free_start;
			for (DWORD i = 0; i < count; i++, entry++) {
				g_PrintStream.print("%08X map %08X", entry, entry->map);
				if (entry->map) {
					GlobalFree(entry->map);
					g_PrintStream << entry->map << endl;
				}
				if (entry->gametype) GlobalFree(entry->gametype);
				if (entry->scripts) {
					for (DWORD x = 0; x < entry->scripts->count; x++)
						GlobalFree(entry->scripts->script_names[x]);
					GlobalFree(entry->scripts);
				}
				g_PrintStream.print("success on %i", i);
			}
		}
		// free the current cycle
		void Free()
		{
			g_PrintStream << "Freeing whole cycle" << endl;
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
			g_PrintStream.print("new alloc %08X (old %08X)", new_cycle, start);
			g_PrintStream.print("copying %i entries", cur_count);
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
			DWORD len = map.size() + 1;
			entry.map = (char*)GlobalAlloc(GMEM_FIXED, len);
			g_PrintStream.print("allocated map memory %08X", entry.map);
			strcpy_s(entry.map, len, map.c_str());

			// Allocate memory for the gametype
			{
				std::string n_gametype = NarrowString(gametype);
				len = n_gametype.size() + 1;
				entry.gametype = (char*)GlobalAlloc(GMEM_FIXED, len);
				strcpy_s(entry.gametype, len, n_gametype.c_str());
			}

			if (scripts.size() > 0) {
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
			} else entry.scripts = NULL;

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
			g_PrintStream << "Cleaning up cycle with " << (DWORD)cur_count << " entires"
				<< endl;
			Free();
			g_PrintStream << "Done" << endl;
		}

		// load the specified game into this cycle, expanding if necessary.
		bool AddGame(s_phasor_mapcycle_entry& game, COutStream& out)
		{
			if (cur_count >= allocated_count) {
				if (!Expand(3)) return false;
			}
			g_PrintStream.print("adding game into %08X", start + cur_count);
			bool success = BuildNewEntry(game.map, game.gametype, game.scripts,
				*(start + cur_count), out);
			if (!success) return false;
			cur_count++;
			return true;
		}

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
		void debug_print()
		{
			s_mapcycle_entry* entry = start;
			for (DWORD i = 0; i < cur_count; i++, entry++) {
				g_PrintStream.print("%08X map %08X", entry, entry->map);
			}
		}

		bool DeleteGame(size_t index)
		{
			if (index < 0 || index >= cur_count) return false;
			s_mapcycle_entry* to_remove = start + index;
			Free(to_remove, 1);
			memcpy(to_remove, to_remove + 1, cur_count - index - 1);
			cur_count--;

			if (active) {
				// Make sure the index Halo's using is still valid
				if (g_mapcycle_header->current >= cur_count)
					g_mapcycle_header->current = -1; // restart cycle
				else
					g_mapcycle_header->current -= 1;
			}

			return true;
		}

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

		void SetActiveCycle(std::unique_ptr<CHaloMapcycle> new_cycle)
		{
			if (active) active->NoLongerActive();
			active = std::move(new_cycle);
			active->Active();
			mapcycle_header.games = active->start;
			mapcycle_header.cur_count = active->cur_count;
			mapcycle_header.allocated_count = active->allocated_count;
			mapcycle_header.current = -1;
		}

		bool is_empty() { return !active; }
		CHaloMapcycle& GetActive() { return *active; }

		friend class CHaloMapcycle;
	};

	std::unique_ptr<CHaloMapcycleLoader> cycle_loader;

	void InitilizeMapcycle()
	{
		s_mapcycle_header* header = (s_mapcycle_header*)ADDR_MAPCYCLELIST;
		cycle_loader = std::unique_ptr<CHaloMapcycleLoader>(new CHaloMapcycleLoader(*header));
	}

	s_mapcycle_entry* GetCurrentMapcycleEntry()
	{
		DWORD index = g_mapcycle_header->current;
		return index >= 0 && index < g_mapcycle_header->cur_count
			? g_mapcycle_header->games + index : NULL;
	}
	
	// Checks if the map, gametype and all scripts are valid.
	bool ValidateUserInput(const std::string& map, const std::string& gametype,
		const std::vector<std::string>& scripts, COutStream& out)
	{
		if (!IsValidMap(map))	{
			out << map << L" isn't a valid map." << endl;
			return false;
		}
		if (!gametypes::IsValidGametype(WidenString(gametype))) {
			out << gametype << L" isn't a valid gametype." << endl;
			return false;
		}

		bool success = true;
		for (size_t x = 0; x < scripts.size(); x++) {
			if (!scriptloader::IsValidScript(scripts[x])) {
				out << scripts[x] << L" isn't a valid script." << endl;
				success = false;
			}
		}
		return success;
	}

	// --------------------------------------------------------------------
	// Mapcycle controlling functions
	// 
	std::vector<s_phasor_mapcycle_entry> mapcycleList;
	bool in_mapcycle = false;

	e_command_result sv_mapcycle_begin(void*, 
		std::vector<std::string>& tokens, COutStream& out)
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
		g_PrintStream.print("before");
		cycle_loader->GetActive().debug_print();
		server::StartGame(mapcycleList[0].map.c_str());
		g_PrintStream.print("after");
		cycle_loader->GetActive().debug_print();
		// getting changed by StartGame.. why?
		// ps. count + 4 is allocated size
		//g_PrintStream << "map count " << GetMapcycleCount() << endl;
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
		if (tokens.size() > 3)
			entry.scripts.assign(tokens.begin() + 3, tokens.end());

		if (!ValidateUserInput(tokens[1], tokens[2], entry.scripts, out))
			return e_command_result::kProcessed;

		entry.map = tokens[1];
		entry.gametype = WidenString(tokens[2]);

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

		mapcycleList.erase(mapcycleList.begin() + index);
		if (in_mapcycle)
			cycle_loader->GetActive().DeleteGame(index);

		// Display cycle as it is now
		sv_mapcycle(exec_player, tokens, out);
		return e_command_result::kProcessed;
	}

	e_command_result sv_mapcycle(void*, 
		std::vector<std::string>& tokens, COutStream& out)
	{
		out.wprint(L"    %-20s%-20s%s", L"Map", L"Variant", L"Script(s)");
		const wchar_t* fmt = L"%-4i%-20s%-20s%s";

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
		std::vector<std::string>& tokens, COutStream& out)
	{
		if (tokens.size() < 3) {
			out << L"Syntax: sv_map <map> <gametype> opt: <script1> <script2> ..."
				<< endl;
			return e_command_result::kProcessed;
		}
		s_phasor_mapcycle_entry entry;
		if (tokens.size() > 3)
			entry.scripts.assign(tokens.begin() + 3, tokens.end());

		if (!ValidateUserInput(tokens[1], tokens[2], entry.scripts, out))
			return e_command_result::kProcessed;
		entry.map = tokens[1];
		entry.gametype = WidenString(tokens[2]);

		std::unique_ptr<CHaloMapcycle> new_cycle = 
			std::unique_ptr<CHaloMapcycle>(new CHaloMapcycle());
		
		if (!new_cycle->AddGame(entry, out)) {
			out << L"Previous errors prevent the game from being started." << endl;
			return e_command_result::kProcessed;
		}
		cycle_loader->SetActiveCycle(std::move(new_cycle));
		
		// todo: disable next map vote
		// 
		
		// start the new game
		server::StartGame(entry.map.c_str());
		in_mapcycle = false;

		return e_command_result::kProcessed;
	}

	e_command_result sv_end_game(void*, std::vector<std::string>&, COutStream&)
	{
		in_mapcycle = false;
		return e_command_result::kGiveToHalo;
	}

	// --------------------------------------------------------------------
	// Initialize the system
	void Initialize(COutStream& out)
	{
#ifdef PHASOR_PC
		BuildMapList(out);
#endif
		InitilizeMapcycle();

	}
}}}