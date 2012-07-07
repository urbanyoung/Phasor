#include <Windows.h>

// Entry point
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	// Avoid the unused parameter warning
	UNREFERENCED_PARAMETER(lpReserved);

	// Check if the DLL is being loaded
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		// Disable calls for DLL_THREAD_ATTACH and DLL_THREAD_DETACH
		DisableThreadLibraryCalls(hModule);
	}

	return TRUE;
}