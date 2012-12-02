#include <windows.h>
#include <stdio.h>
#include "Phasor/Logging.h"
#include "Phasor/Directory.h"
#include "Scripting.h"

std::unique_ptr<CLoggingStream> g_ScriptsLog;
std::unique_ptr<CLoggingStream> g_PhasorLog;

// Locate and create all directories Phasor will use. If an error occurs
// this function never returns and the process is terminated.
void LocateDirectories();

// Called when the dll is loaded
//extern "C" __declspec(dllexport) void OnLoad()
int main()
{
	printf("44656469636174656420746f206d756d2e2049206d69737320796f752e\n");
	LocateDirectories();

	try
	{
		g_PhasorLog.reset(new CLoggingStream(g_LogsDirectory, L"PhasorLog"));
		//CLoggingStream g_PhasorLog(g_LogsDirectory, L"PhasorLog");
		*g_PhasorLog << "test" << endl;
	}
	catch (...)
	{

	}
	
}

class CEarlyError : public CLoggingStream
{
private:
	virtual bool Write(const std::wstring& str)
	{
		wprintf(L"%s\n", str.c_str());
		CLoggingStream::Write(str);
		Sleep(10000);
		exit(1);
		return true; // no return
	}

public:
	CEarlyError (const std::wstring& file)
		: CLoggingStream(file)
	{
	}

	virtual ~CEarlyError() {}
};

// Locate and create all directories Phasor will use. If an error occurs
// this function never returns and the process is terminated.
void LocateDirectories()
{
	CEarlyError earlyerror(L"earlyerror");
	try 
	{
		SetupDirectories();
	}
	catch (std::exception & e)
	{
		earlyerror << L"An error occurred which prevented Phasor from loading : "
			<< e.what() << endl;
	}	
	catch (...)
	{
		earlyerror << "An unknown error occurred which prevented Phasor from loading" << endl;
	}
}

// Windows entry point
/*BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		// Disable calls for DLL_THREAD_ATTACH and DLL_THREAD_DETACH
		DisableThreadLibraryCalls(hModule);
	}

	return TRUE;
}*/