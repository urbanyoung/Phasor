#include "ServerStreams.h"
#include "../Addresses.h"
#include <iostream>

halo::server::CHaloPrintStream g_PrintStream;

namespace halo { namespace server 
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

	bool CHaloEchoStream::Write(const std::wstring& str)
	{
		printStream.Write(str);
		logStream << str;
		logStream.Flush();
		return true;
	}
}}