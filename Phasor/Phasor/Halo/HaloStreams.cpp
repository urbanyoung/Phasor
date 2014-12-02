#include "HaloStreams.h"
#include "Addresses.h"
#include "Server/Server.h"
#include "Game/Game.h"
#include <iostream>
#include "../../Common/MyString.h"

namespace halo
{
#ifdef PHASOR_PC
	const std::wstring MSG_PREFIX = L"** SERVER ** ";
#elif PHASOR_CE
	const std::wstring MSG_PREFIX;
#endif

	bool CHaloPrintStream::Write(const std::wstring& str)// str usually has endl appended
	{
		if (str.size() == 0) return true;
		
		bool ready = *(bool*)UlongToPtr(ADDR_CONSOLEREADY);
		if (!ready) {
			std::wcout << str;
			return true;
		}
		// Prepare for writing the string
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD written = 0;

		CONSOLE_SCREEN_BUFFER_INFO info;
		SHORT oldX = 0; // used to set cursor back to old position

		// Get current console position info
		GetConsoleScreenBufferInfo(hConsole, &info);
		oldX = info.dwCursorPosition.X;

		// Set cursor to start of the last row (where we want to start writing)
		info.dwCursorPosition.X = 0;
		info.dwCursorPosition.Y = 299;
		SetConsoleCursorPosition(hConsole, info.dwCursorPosition);

		FillConsoleOutputCharacterA(hConsole, ' ', 95, info.dwCursorPosition, &written);
		FillConsoleOutputAttribute(hConsole, 7, 95, info.dwCursorPosition, &written);

		// Write the text
		WriteConsoleW(hConsole, str.c_str(), str.size(), &written, NULL);
		//WriteConsoleW(hConsole, L"\n", 1, &written, NULL);

		// Get the current text in the console
		LPBYTE ptr = (LPBYTE)ADDR_CONSOLEINFO;

		if (*ptr != 0) {
			// Build current command input
			std::string formatted = "halo( ";

			formatted += (char*)UlongToPtr(*(DWORD*)ptr + OFFSET_CONSOLETEXT); // current text

			// Rewrite the data to console
			GetConsoleScreenBufferInfo(hConsole, &info);
			FillConsoleOutputCharacterA(hConsole, ' ', 95, info.dwCursorPosition, &written);
			WriteConsoleOutputCharacterA(hConsole, formatted.c_str(), formatted.size(), info.dwCursorPosition, &written);

			// Set the cursor to its old position
			GetConsoleScreenBufferInfo(hConsole, &info);
			info.dwCursorPosition.X = oldX;
			SetConsoleCursorPosition(hConsole, info.dwCursorPosition);
		}
		return true;
	}

	// --------------------------------------------------------------------
	//
	CPlayerBaseStream::CPlayerBaseStream(const s_player& player, bool do_check) 
		: player(player), memory_id(player.memory_id), hash(player.hash)
	{}
	CPlayerBaseStream::CPlayerBaseStream(const s_player& player)
		: player(player), memory_id(-1)
	{}

	bool CPlayerBaseStream::ValidatePlayer()
	{
		if (memory_id != -1) {
			// If the stream gets cloned it creates a checked stream.
			// so we need to check that CPlayerStream references the correct
			// slot and that the hashes match.
			// GetPlayer will always return the same ptr for the same 
			// slot so we only need to check if its non-null
			s_player* player = game::getPlayer(memory_id);
			if (player != &this->player || player->hash != hash) 
				return false; // player is now invalid. ignore.
		}
		return true;
	}

	// -------------------------------------------------------------------
	//

	PlayerConsoleStream::PlayerConsoleStream(const s_player& player)
		: CPlayerBaseStream(player) 
	{
	}

	//use checking
	PlayerConsoleStream::PlayerConsoleStream(const s_player& player, bool)
		: CPlayerBaseStream(player, true)
	{
	}

	bool PlayerConsoleStream::Write(const std::wstring& str)
	{
		if (!ValidatePlayer()) return true; // invalid now
		server::ConsoleMessagePlayer(player, StripTrailingEndl(str));
		return true;
	}

	// -------------------------------------------------------------------
	//
	PlayerChatStreamRaw::PlayerChatStreamRaw(const s_player& player)
		: CPlayerBaseStream(player) 
	{
	}

	//use checking
	PlayerChatStreamRaw::PlayerChatStreamRaw(const s_player& player, bool)
		: CPlayerBaseStream(player, true)
	{
	}

	bool PlayerChatStreamRaw::Write(const std::wstring& str)
	{
		if (!ValidatePlayer()) return true; // invalid now
		server::MessagePlayer(player, StripTrailingEndl(str));
		return true;
	}

	PlayerChatStream::PlayerChatStream(const s_player& player)
		: PlayerChatStreamRaw(player) 
	{
	}

	//use checking
	PlayerChatStream::PlayerChatStream(const s_player& player, bool)
		: PlayerChatStreamRaw(player, true)
	{
	}

	bool PlayerChatStream::Write(const std::wstring& str)
	{
		return PlayerChatStreamRaw::Write(MSG_PREFIX + str);
	}
}