#include <windows.h>
#include <stdio.h>
#ifdef BUILD_DEBUG
//#include <vld.h>
#endif
#include "Phasor/Logging.h"
#include "Phasor/ThreadedLogging.h"
#include "Phasor/GameLogging.h"
#include "Phasor/Directory.h"
#include "Scripting.h"
#include "Phasor/Commands.h"
#include "Phasor/Admin.h"
#include "Phasor/Halo/Addresses.h"
#include "Phasor/Halo/Hooks.h"
#include "Phasor/Halo/Alias.h"
#include "Phasor/Halo/Server/MapLoader.h"
#include "Phasor/Halo/Server/Gametypes.h"
#include "Phasor/CrashHandler.h"
#include "Phasor/Globals.h"
#include "Common/FileIO.h"

#define WAIT_AND_QUIT Sleep(10000); exit(1);
//#define WAIT_AND_QUIT Sleep(10000); return 1;

// Globals through Phasor's lifetime
PhasorThread g_Thread; // must be above all other objects
Timers g_Timers;
std::unique_ptr<CScriptsLog> g_ScriptsLog;
std::unique_ptr<CPhasorLog> g_PhasorLog;
std::unique_ptr<CGameLog> g_GameLog;
std::unique_ptr<CRconLog> g_RconLog;
std::unique_ptr<Scripting::Scripts> g_Scripts;

// Locate and create all directories Phasor will use. If an error occurs
// this function never returns and the process is terminated.
void LocateDirectories();

// Process any early Phasor commands.
void LoadEarlyInit(COutStream& out);

// Called when the dll is loaded
extern "C" __declspec(dllexport) void OnLoad()
//int main()
{
	g_PrintStream << L"44656469636174656420746f206d756d2e2049206d69737320796f752e" << endl;
	LocateDirectories();

	// can't rename phasor log for startup errors via earlyinit
	CLoggingStream PhasorLog(g_LogsDirectory, L"PhasorLog", g_OldLogsDirectory);

	try
	{
		PhasorLog << L"Initializing Phasor ... " << endl;	

		PhasorLog << L"Locating Halo addresses and structures" << endl;
		DWORD ticks = GetTickCount();
		Addresses::LocateAddresses();
		//PhasorLog << L"Finished in " << GetTickCount() - ticks << " ticks" << endl;
		
		PhasorLog << L"Installing crash handler..." << endl;
		CrashHandler::InstallCatchers();

//#ifdef PHASOR_PC
//		halo::server::maploader::BuildMapList(PhasorLog);
//#endif
		halo::server::maploader::Initialize(PhasorLog);
		PhasorLog << L"Building gametype list..." << endl;
		if (!halo::server::gametypes::BuildGametypeList())
			PhasorLog << L"    No gametypes were found!" << endl;
		PhasorLog.print("%i", (int)0x80000000);
		halo::InstallHooks();

		if (!g_Thread.run()) {
			throw std::exception("cannot start the auxiliary thread.");
		}

		// Initialize the other logs
		g_PhasorLog.reset(new CThreadedLogging(PhasorLog, g_Thread, 0));
		g_ScriptsLog.reset(new CThreadedLogging(
			g_LogsDirectory, L"ScriptsLog", g_OldLogsDirectory, g_Thread, 0));
	//	g_ScriptsLog->EnableTimestamp(false);
		g_GameLog.reset(new CGameLog(g_LogsDirectory, L"GameLog", g_Thread));
		g_RconLog.reset(new CThreadedLogging(g_LogsDirectory, L"RconLog", g_OldLogsDirectory, g_Thread));
		g_Scripts.reset(new Scripting::Scripts(*g_ScriptsLog,g_ScriptsDirectory));

		PhasorLog << L"Processing earlyinit.txt" << endl;
		LoadEarlyInit(PhasorLog);

		PhasorLog << L"Initializing admin system" << endl;
		Admin::Initialize(&PhasorLog);

		PhasorLog << L"Initializing alias system" << endl;
		halo::alias::Initialize();

		PhasorLog << L"Phasor was successfully initialized." << endl;

		// We want threaded logging from now on
		*g_PhasorLog << "test" << endl;		
	}
	catch (std::exception& e)
	{
		PhasorLog << "Phasor cannot be loaded because : " <<  e.what() << endl;
		g_PrintStream << "Phasor cannot be loaded because : " <<  e.what() << endl;
		WAIT_AND_QUIT
	}
	catch (...)
	{
		static const std::wstring err = L"An unknown error occurred which prevented Phasor from loading";
		PhasorLog << err << endl;
		g_PrintStream << err << endl;
		WAIT_AND_QUIT
	}
	
	/*thread.close();

	while (!thread.has_closed()) {
		Sleep(10);
	}	*/
}

// Locate and create all directories Phasor will use. If an error occurs
// this function never returns and the process is terminated.
void LocateDirectories()
{
	try 
	{
		SetupDirectories();
	}
	catch (std::exception & e)
	{
		g_PrintStream << L"An error occurred which prevented Phasor from loading : "
			<< e.what() << endl;
		WAIT_AND_QUIT
	}	
}

void LoadEarlyInit(COutStream& out)
{
	CInFile file;
	if (!file.Open(L"earlyinit.txt"))
		return; // may not exist

	char line[4096];
	while (file.ReadLine<char>(line, NELEMS(line), NULL)) {
		commands::ProcessCommand(line, out);
		g_PrintStream << line << endl;
	}
}

// Windows entry point
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		// Disable calls for DLL_THREAD_ATTACH and DLL_THREAD_DETACH
		DisableThreadLibraryCalls(hModule);
	}

	return TRUE;
}