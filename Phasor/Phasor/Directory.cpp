#include "Directory.h"
#include "../Common/MyString.h"
#include "../Common/FileIO.h"
#include <windows.h>
#include <vector>

std::wstring g_ProfileDirectory;
std::wstring g_DataDirectory;
std::wstring g_CrashDirectory;
std::wstring g_ScriptsDirectory;
std::wstring g_LogsDirectory;
std::wstring g_OldLogsDirectory;
std::wstring g_MapDirectory;

void ParseCommandLine(const std::wstring& commandline, // first is exe name
	std::wstring& dataPath, std::wstring& mapPath, std::wstring& scriptPath);

void CreateSubDirectory(const std::wstring& name, std::wstring& out, 
	const std::wstring& relative = g_ProfileDirectory)
{
	out = relative + name;
	if (!NDirectory::CreateDirectory(out)) {
		std::string err = "The " + NarrowString(name) + " directory (" + 
			NarrowString(out) +	") couldn't be created.";
		throw std::exception(err.c_str());
	}
	NDirectory::NormalizeDirectory(out);
}

void SetupDirectories()
{
	ParseCommandLine(GetCommandLineW(), g_ProfileDirectory, g_MapDirectory, g_ScriptsDirectory);

	if (g_ProfileDirectory.empty())
	{
		if (!NDirectory::GetMyDocuments(g_ProfileDirectory))
			throw std::exception("can't determine path to My Documents. Consider using -path command line switch.");
#ifdef PHASOR_PC
		g_ProfileDirectory += L"\\My Games\\Halo";
#elif PHASOR_CE
		g_ProfileDirectory += L"\\My Games\\Halo CE";
#endif
		if (!NDirectory::IsDirectory(g_ProfileDirectory)) {
			std::string err = NarrowString(g_ProfileDirectory) + " is not a valid directory.";
			throw std::exception(err.c_str());
		}
	}

	NDirectory::NormalizeDirectory(g_ProfileDirectory);
	NDirectory::NormalizeDirectory(g_MapDirectory);
	if (g_ScriptsDirectory.size()) NDirectory::NormalizeDirectory(g_ScriptsDirectory);

	CreateSubDirectory(L"data", g_DataDirectory);
	if (!g_ScriptsDirectory.size()) CreateSubDirectory(L"scripts", g_ScriptsDirectory);
	CreateSubDirectory(L"logs", g_LogsDirectory);
	CreateSubDirectory(L"old", g_OldLogsDirectory, g_LogsDirectory);
	CreateSubDirectory(L"crash", g_CrashDirectory);
}

void ParseCommandLine(const std::wstring& commandline, // first is exe name
	std::wstring& dataPath, std::wstring& mapPath, std::wstring& scriptPath)
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
			} else if (tokens[x] == L"-scriptpath") {
				NDirectory::NormalizeDirectory(tokens[x+1]);
				if (NDirectory::IsDirectory(tokens[x+1])) scriptPath = tokens[x+1];
				x++;
			}
		}
	}
}