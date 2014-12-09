#include <vector>
#include <Windows.h>

//-------------------------------------------------------------------------------------------------
// Events
//

// Called when the DLL has loaded successfully
void __stdcall OnLoad()
{
	// Get address of entry point
	LPBYTE lpModule = (LPBYTE)GetModuleHandle(0);
	DWORD dwOffsetToPE = *(DWORD*)(lpModule + 0x3C);
	DWORD dwOffsetToEP = *(DWORD*)(lpModule + dwOffsetToPE + 0x28);
	LPBYTE lpEntryPoint = lpModule + dwOffsetToEP;

	// Check if first instruction matches the halo server
	// PUSH 0x18 is halo server
	// PUSH 0x60 is halo client
	if (*(WORD*)lpEntryPoint == 0x186A)
	{
		// Load Phasor.dll
		//HMODULE hModule = LoadLibrary("PhasorCE.dll");
		HMODULE hModule = LoadLibrary("E:\\Development\\C++\\Phasor\\Debug\\Phasor.dll");

		// Check if Phasor loaded successfully
		if (hModule)
		{
			// Call Phasors OnLoad event
			typedef void (__cdecl *PhasorOnLoad)();
			PhasorOnLoad OnLoad = (PhasorOnLoad)GetProcAddress(hModule, "OnLoad");
			OnLoad();
		}
	}
}

//
//-------------------------------------------------------------------------------------------------
// Memory Functions
//

// Writes to process memory
BOOL WriteBytes(DWORD dwAddress, LPVOID lpBuffer, DWORD dwCount)
{
	BOOL bResult = TRUE;

	HANDLE hProcess = GetCurrentProcess();
	LPVOID lpAddress = UlongToPtr(dwAddress);

	DWORD dwOldProtect;
	DWORD dwNewProtect = PAGE_EXECUTE_READWRITE;

	bResult &= VirtualProtect(lpAddress, dwCount, dwNewProtect, &dwOldProtect);		// Allow read/write access
	bResult &= WriteProcessMemory(hProcess, lpAddress, lpBuffer, dwCount, NULL);	// Write to process memory
	bResult &= VirtualProtect(lpAddress, dwCount, dwOldProtect, &dwNewProtect);		// Restore original access
	bResult &= FlushInstructionCache(hProcess, lpAddress, dwCount);					// Update instruction cache

	return bResult;
}

// Finds all locations of a signature
std::vector<DWORD> FindSignature(LPBYTE lpBuffer, DWORD dwBufferSize, LPBYTE lpSignature, DWORD dwSignatureSize, LPBYTE lpWildCards = 0)
{
	// List of locations of the signature
	std::vector<DWORD> addresses;

	// Loop through each byte in the buffer
	for (DWORD i = 0; i < dwBufferSize; ++i)
	{
		bool bFound = true;

		// Loop through each byte in the signature
		for (DWORD j = 0; j < dwSignatureSize; ++j)
		{
			// Check if the index overruns the buffer
			if (i + j >= dwBufferSize)
			{
				bFound = false;

				break;
			}

			// Check if wild cards are used
			if (lpWildCards)
			{
				// Check if the buffer does not equal the signature and a wild card is not set
				if (lpBuffer[i + j] != lpSignature[j] && !lpWildCards[j])
				{
					bFound = false;

					break;
				}
			}
			else
			{
				// Check if the buffer does not equal the signature
				if (lpBuffer[i + j] != lpSignature[j])
				{
					bFound = false;

					break;
				}
			}
		}

		// Check if the signature was found and add it to the address list
		if (bFound)
			addresses.push_back(i);
	}

	// Return the list of locations of the signature
	return addresses;
}

// Finds the location of a signature
DWORD FindAddress(LPBYTE lpBuffer, DWORD dwBufferSize, LPBYTE lpSignature, DWORD dwSignatureSize, LPBYTE lpWildCards = 0, DWORD dwIndex = 0, DWORD dwOffset = 0)
{
	DWORD dwAddress = 0;

	// Find all locations of the signature
	std::vector<DWORD> addresses = FindSignature(lpBuffer, dwBufferSize, lpSignature, dwSignatureSize, lpWildCards);

	// Return the requested address
	if (addresses.size() - 1 >= dwIndex)
		dwAddress = (DWORD)lpBuffer + addresses[dwIndex] + dwOffset;

	return dwAddress;
}

//
//-------------------------------------------------------------------------------------------------
// Code Caves
//

DWORD dwReturn;

__declspec(naked) void OnLoad_CC()
{
	__asm
	{
		// Store return address
		pop dwReturn

		// Store registers and flags
		pushad
		pushfd

		// Call the event
		call OnLoad

		// Restore registers and flags
		popfd
		popad

		// Execute original code
		LEA EDX,DWORD PTR SS:[ESP+8]
		PUSH EDX

		// Restore the return address
		push dwReturn

		// Return to original code
		ret
	}
}

// Creates a code cave to a function at a specific address
BOOL CreateCodeCave(DWORD dwAddress, BYTE cbSize, VOID (*pFunction)())
{
	BOOL bResult = TRUE;

	if (cbSize < 5)
		return FALSE;

	// Calculate the offset from the function to the address
	DWORD dwOffset = PtrToUlong(pFunction) - dwAddress - 5;

	// Construct the call instruction to the offset
	BYTE patch[0xFF] = {0x90};
	patch[0] = 0xE8;
	memcpy(patch + 1, &dwOffset, sizeof(dwAddress));

	// Write the code cave to the address
	bResult &= WriteBytes(dwAddress, patch, cbSize);

	return bResult;
}

//
//-------------------------------------------------------------------------------------------------
// Entry Point
//

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	// Avoid the unused parameter warning
	UNREFERENCED_PARAMETER(lpReserved);

	// Check if the DLL is being loaded
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		// Disable calls for DLL_THREAD_ATTACH and DLL_THREAD_DETACH
		DisableThreadLibraryCalls(hModule);

		// Get address of code section
		LPBYTE lpModule = (LPBYTE)GetModuleHandle(0);
		DWORD dwOffsetToPE = *(DWORD*)(lpModule + 0x3C);
		DWORD dwSizeOfCode = *(DWORD*)(lpModule + dwOffsetToPE + 0x1C);
		DWORD dwBaseOfCode = *(DWORD*)(lpModule + dwOffsetToPE + 0x2C);
		LPBYTE lpCode = (LPBYTE)(lpModule + dwBaseOfCode);

		// Find the address of the OnLoad signature
		BYTE sig[] = {0x8D, 0x54, 0x24, 0x08, 0x52, 0x68, 0x19, 0x00, 0x02, 0x00, 0x6A, 0x00};
		DWORD cc_onload = FindAddress(lpCode, dwSizeOfCode, sig, sizeof(sig));

		// Check if the signature was found and create the code cave
		if (cc_onload)
			CreateCodeCave(cc_onload, 5, OnLoad_CC);
	}

	return TRUE;
}

//
//-------------------------------------------------------------------------------------------------