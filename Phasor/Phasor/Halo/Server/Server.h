//! \file Server.h
#pragma once

#include "../../../Common/Types.h"
#include "MapLoader.h"

// declared in ../../Commands.h
enum e_command_result;

namespace halo { 
	struct s_player;
	struct s_player_structure;
namespace server
{
	//---------------------------------------------------------------------
	// DEFINITIONS
	// 
	#pragma pack(push, 1)
	//! Represents Halo's connection information structure (ip, port, keys etc)
	struct s_connection_info
	{
		BYTE ip[4];
		WORD port;
		WORD pad;
		BYTE unk[0x148]; // handshake stuff etc, don't care atm.
	};
	static_assert(sizeof(s_connection_info) == 0x150, "incorrect s_connection_info");

	//! Represents an entry in Halo's machine table
	struct s_machine_info
	{
		s_connection_info*** con_info_ptr; 
		DWORD zero;
		DWORD zero1;
		WORD playerNum; // used for rcon etc
		WORD seven;
		BYTE unk[0x42];
		char key[10]; // only 7 chars long tho.. i think rest is padding
		DWORD machineNum; // 0 - 0xFFFFFFFF increased for each connection in server's life

		s_connection_info* get_con_info()
		{
			return **con_info_ptr;
		}
#ifdef PHASOR_CE
		char unk1[0x20];
		char ip[0x20];
		char cd_key_hash[0x20];
		char unk2[0x2c];
#endif
	};
	static_assert(sizeof(s_machine_info) == MACHINE_ENTRY_SIZE, "incorrect s_machine_info");

	//! Represents an entry in Halo's connection player table
	struct s_presence_item
	{
		wchar_t name[12];
		DWORD idk;
		BYTE machineId;
		BYTE status; // 1 = ok, 2 = invalid hash (or auth, or w/e)
		BYTE team;
		BYTE playerId;
	};
	static_assert(sizeof(s_presence_item) == 0x20, "incorrect s_presence_item");

    struct s_previous_ping
    {
        BYTE memoryId;
        UNKNOWN(3); // padding
        DWORD ping;
    };
    static_assert(sizeof(s_previous_ping) == 0x08, "incorrect s_previous_ping");

	//! Server related items (name, gametype, players, machines)
	struct s_server_info
	{
		void* unk_ptr;
		WORD state; // 0 inactive, 1 game, 2 results
		WORD unk1;
		wchar_t server_name[0x42];
		char map_name[0x80];
		wchar_t gametype[0x18];
		BYTE unk2[0x69];
#ifdef PHASOR_CE
		BYTE unk_ce[0x40];
#endif
		BYTE max_players;//1a5 pc, 1e5 ce
		WORD unk3; // 1a6, 1e6
		BYTE cur_players; // 1a8, 1e8
		BYTE unk4; // 1a9, 1e9
		s_presence_item player_data[16]; // 1aa, 1ea
		BYTE unk5[14]; // 3aa, 3ea
		s_machine_info machine_table[16]; // 3b8, 3f8
        UNKNOWN(8); // 9b8, 12b8
        DWORD last_ping_request; // timestamp 9c0, 12c0
        UNKNOWN(0x5C); // 9c4, 12c4
        s_previous_ping previous_pings[16]; // a20, 1320

	};

	struct s_command_input {
		char password[9]; // don't forget to ensure null-termination
		char command[65];
	};
	static_assert(sizeof(s_command_input) == 74, "incorrect s_command_input");

	struct s_hash_validation {
		DWORD machineId;
		char hash[32];
		DWORD empty;
		DWORD requestId;
		UNKNOWN(0x0C);
		DWORD status; // 1 = valid, 2 = invalid
	};

	struct s_machinfo_info_partial_packet
	{
		char hash[32];
		char challenge[32];
		char clientKey[9]; // including null terminator
		UNKNOWN(1); // only seen 1, halo uses it for something 
		UNKNOWN(2); //probably padding
		wchar_t name[12]; // including null terminator		
	};

	#pragma pack(pop)

	struct PhasorMachine
	{
		s_machine_info* machine;
		bool hash_validated;

		PhasorMachine(s_machine_info* machine);
		~PhasorMachine();
	};

	class SayStreamRaw : public COutStream
	{
	protected:
		virtual bool Write(const std::wstring& str);

	public:
		SayStreamRaw() {}

		virtual std::unique_ptr<COutStream> clone() const override
		{
			return std::unique_ptr<COutStream>(new SayStreamRaw());
		}
	};

	// Stream for sending server messages with **SERVER** prepended
	class SayStream : public SayStreamRaw
	{
	protected:
		virtual bool Write(const std::wstring& str);

	public:
		SayStream() {}

		virtual std::unique_ptr<COutStream> clone() const override
		{
			return std::unique_ptr<COutStream>(new SayStream());
		}
	};

	//! Stream used for server messages.
	extern SayStream say_stream;
	extern SayStreamRaw say_stream_raw;

	void acceptInvalidHashes(bool state);
	bool getInvalidHashState();

	/*! \brief Send a chat message to the player
	 *	\param player The player to send the message to
	 *	\param str The message to send.	 */
	void MessagePlayer(const s_player& player, const std::wstring& str);
	
	/*! \brief Send a console message to the player
	 *	\param player The player to send the message to
	 *	\param str The message to send.	 */
	bool ConsoleMessagePlayer(const s_player& player, const std::wstring& str);
	
	/*! \brief Notifies the server that a player has changed team (syncs it)
	 *	\param player The player who changed team. */
	void NotifyServerOfTeamChange(const halo::s_player& player);

	void ExecuteServerCommand(const std::string& command, s_player* execute_as);

	// Gets the player's ip
	bool GetPlayerIP(const s_player& player, std::string* ip, WORD* port);
	bool GetMachineIP(s_machine_info& machine, std::string* ip, WORD* port);
	
	// Gets the player's hash
	bool GetPlayerHash(const s_player& player, std::string& hash);
	bool GetMachineHash(const s_machine_info& machine, std::string& hash);
	
	PhasorMachine* FindMachine(const s_player& player);
	PhasorMachine* FindMachineById(DWORD machineId);
	PhasorMachine* FindMachineByIndex(DWORD index);

	/*! \brief Get the player executing the current command
	 * \return The player executing the command, or NULL if no player. */
	halo::s_player* GetPlayerExecutingCommand();

	void SetExecutingPlayer(halo::s_player* player);

	/*! \brief Get the server struct
		\return The server struct.*/
	s_server_info* GetServerStruct();

	DWORD GetServerTicks();
	DWORD GetRespawnTicks();

	e_command_result sv_quit(void*, 
		commands::CArgParser&, COutStream&);

	// --------------------------------------------------------------------
	// Events
	// 

	void __stdcall OnMachineConnect(DWORD machineIndex);
	void __stdcall OnMachineDisconnect(DWORD machineIndex);
	void __stdcall OnMachineInfoFix(s_machinfo_info_partial_packet* data);
	
	// Called for console events (exit etc)
	/*! \brief Called for Windows related console events (ie closing the server)
	 *	\param fdwCtrlType The type of event. */
	void __stdcall ConsoleHandler(DWORD fdwCtrlType);

	/*! \brief Called every cycle to read input from the user. I use it for timers. */
	void __stdcall OnConsoleProcessing();

	/*! \brief Called when a client sends its update packet (new pos, fire etc)
	 *	\param m_player The memory address of the player who is updating.*/
	void __stdcall OnClientUpdate(s_player_structure* m_player);

	/*! \brief Called to process a server command, after the password has been validated.
	 *	\param command The command being executed.
	 *	\return Value indicating whether or not Halo should process the event.*/
	e_command_result __stdcall ProcessCommand(char* command);

	void __stdcall ProcessCommandAttempt(s_command_input* input, int playerNum);

	/*! \brief Called when a new game starts.
	 *	\param map The map the game is running.*/
	void __stdcall OnNewGame(const char* map);

	/*! \brief Called when a game stage ends.
	 *	\param mode Specifies which stage is ending (game, post-game, scorecard) */
	void __stdcall OnGameEnd(DWORD mode);

	/*! \brief Called when a map is being loaded.
	 *	\param loading_map The map being loaded, which can be changed.
	 *	\return Boolean indicating whether or not the map was changed.*/
	bool __stdcall OnMapLoad(maploader::s_mapcycle_entry* loading_map);

	/*! \brief Called when halo wants to print a message to the console.*/
	void __stdcall OnHaloPrint(char* msg);

	/*! \brief Called when halo checks if the specified hash is banned.
	 *	\param hash The hash being checked.
	 *	\return Boolean indicating whether or not the player is allowed to join.*/
	bool __stdcall OnHaloBanCheck(char* hash, s_machine_info* machine);

	// Called once Halo has received the hash-checking response from gamespy
	void __stdcall OnHashValidation(s_hash_validation* info, const char* status);

	// Called when the server info is about to be broadcast
	//bool __stdcall OnVersionBroadcast(DWORD arg1, DWORD arg2);

} }