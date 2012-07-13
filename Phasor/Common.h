#pragma once

#include <windows.h>
#include <string>

namespace Common
{
	// Writes to process memory
	BOOL WriteBytes(DWORD dwAddress, LPVOID lpBuffer, DWORD dwCount);

	// Reads from process memory
	BOOL ReadBytes(DWORD dwAddress, LPVOID lpBuffer, DWORD dwCount);

	std::string NarrowString(std::wstring&);
	std::wstring WidenString(std::string&);
}
