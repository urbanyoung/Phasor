#include "HaloStreams.h"
#include "Addresses.h"
#include "Server/Server.h"
#include "Game/Game.h"
#include <iostream>

halo::CHaloPrintStream g_PrintStream;

namespace halo
{
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
	bool CEchoStream::Write(const std::wstring& str)
	{
		bool b1 = first.Write(str);
		return second.Write(str) && b1;
	}

	// -------------------------------------------------------------------
	//
	bool CPlayerStream::Write(const std::wstring& str)
	{
		server::MessagePlayer(player, str);
		return true;
	}

	// --------------------------------------------------------------------
	// 
	// Checks if the specified player (hash and slot) still exist before
	// writing to the player.
	class CCheckedPlayerStream : public CPlayerStream
	{
	private:
		// we need copys of the values, because CPlayerStream::player
		// isn't guaranteed to be valid.
		int memory_id;
		std::string hash;

		void init(const s_player& player)
		{
			memory_id = player.memory_id;
			hash = player.hash;
		}

	protected:

		virtual bool Write(const std::wstring& str) override
		{
			// CPlayerStream expects its player to be valid throughout its
			// lifetime. This may not be true if the stream has been cloned
			// so we need to check that CPlayerStream references the correct
			// slot and that the hashes match.
			// GetPlayer will always return the same ptr for the same 
			// slot so we only need to check if its non-null
			s_player* player = game::GetPlayer(memory_id);
			if (player == &(this->player) && player->hash == hash) 
				return CPlayerStream::Write(str);
			return true;
		}

	public:
		CCheckedPlayerStream(const s_player& player) : CPlayerStream(player)
		{
			init(player);
		}

		CCheckedPlayerStream(CPlayerStream& stream) 
			: CPlayerStream(stream.GetPlayer())
		{
			init(player);
		}

		std::unique_ptr<COutStream> clone() override
		{
			return std::unique_ptr<COutStream>(new CCheckedPlayerStream(player));
		}
	};

	// --------------------------------------------------------------------
	//
	std::unique_ptr<COutStream> CCheckedStream::clone_stream()
	{
		if (player_stream) {
			CPlayerStream& pstream = (CPlayerStream&)stream;
			return std::unique_ptr<COutStream>(
				new CCheckedPlayerStream(pstream));
		} else return stream.clone();
	}
}