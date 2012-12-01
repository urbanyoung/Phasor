#include <windows.h>
#include <stdio.h>
#include "Phasor/Logging.h"
#include "Scripting.h"
#include "Common/MyString.h"
#include "Common/FileIO.h"

std::unique_ptr<CLoggingStream> g_ScriptsLog;

void ParseCommandLine(const std::wstring& input,
	std::wstring& dataPath, std::wstring& mapPath);

// Entry point
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID)
{
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
	// probably won't be able to save the log (can't write to program files)
	// but oh well.
	CLoggingStream earlyerror(L"earlyerror");
	try 
	{
		//using namespace Common;
		printf("44656469636174656420746f206d756d2e2049206d69737320796f752e\n");

		std::wstring dataPath, mapPath;
		ParseCommandLine(GetCommandLineW(), dataPath, mapPath);
	
		throw std::exception("sad face");
		//g_ScriptsLog(new CLoggingStream(

	/*	if (!Phasor::SetupDirectories()) {
			std::string last_err;
			GetLastErrorAsText(last_err);
			printf("Phasor was unable to setup the required directories\n");
			printf("LastError details: %s\n", last_err.c_str());
			return;
		}*/
	}
	catch (std::bad_alloc&)
	{
		earlyerror << L"Phasor was unable to load because sufficient memory wasn't available"
			<< endl;
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

void ParseCommandLine(const std::wstring& commandline, // first is exe name
	std::wstring& dataPath, std::wstring& mapPath)
{
	using namespace std;
	vector<wstring> tokens = TokenizeWArgs(commandline);
	if (tokens.size() < 2)	return;

	for (size_t x = 1; x < tokens.size(); x++) {
		if (x + 1 < tokens.size()) {
			if (tokens[x] == L"-path") {
				NDirectory::NormalizeDirectory(tokens[x+1]);
				if (NDirectory::IsDirectory(tokens[x+1])) dataPath = tokens[x+1];
				x++;
			} else if (tokens[x] == L"-mappath") {
				NDirectory::NormalizeDirectory(tokens[x+1]);
				if (NDirectory::IsDirectory(tokens[x+1])) mapPath = tokens[x+1];
				x++;
			}
		}
	}
}