#include <Windows.h>
#include <stdio.h>

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

// Called when the dll is loaded
extern "C" __declspec(dllexport) void OnLoad()
{
	printf("44656469636174656420746f206d756d2e2049206d69737320796f752e\n");
}