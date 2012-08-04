#include "Phasor.h"
#include "Common.h"
#include <vector>
#include <windows.h>
#include <Shlobj.h>

namespace Phasor
{
	const char* ErrorStream::STREAM_SYNC_MUTEX = "Phasor_Halo_PC_Error_StreamMUTEX";

	ErrorStream::ErrorStream(const std::string& file)
	{
		this->file = file;
		pFile = CreateFile(file.c_str(), GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL);
		if (pFile == INVALID_HANDLE_VALUE) { 
			std::string err = Common::m_sprintf_s(
				"The file to be used as an error stream cannot be accessed: %s", 
				file.c_str());
			throw std::exception(err.c_str());
		}
		fMutex = CreateMutex(NULL, FALSE, STREAM_SYNC_MUTEX);
		if (fMutex == NULL)	{
			CloseHandle(pFile);
			std::string err = Common::m_sprintf_s(
				"Cannot access error stream mutex, error code %i\n",
				GetLastError());
			throw std::exception(err.c_str());
		}
	}

	ErrorStream::~ErrorStream()
	{
		CloseHandle(fMutex);
		CloseHandle(pFile);		
	}

	ErrorStreamPtr ErrorStream::Create(const std::string& file)
	{
		return ErrorStreamPtr(new ErrorStream(file));
	}

	void ErrorStream::Write(const char* _Format, ...)
	{
		va_list ArgList;
		va_start(ArgList, _Format);
		std::string str = Common::FormatVarArgs(_Format, ArgList) + "\n";
		va_end(ArgList);

		// the file can be shared between multiple processes so we need to get a lock
		WaitForSingleObject(fMutex, 1000); // wait 1s then do it anyway
		DWORD bytesWritten = 0;
		WriteFile(pFile, str.c_str(), str.size(), &bytesWritten, NULL);
		ReleaseMutex(fMutex);
	}

	// --------------------------------------------------------------------
	// 
	// Directory stuff
	std::string working_dir, map_dir, data_dir, script_dir, log_dir;
	bool SetupDirectories()
	{
		using namespace Common;
		char* cmdline = GetCommandLineA();

		// Tokenize the command line
		if (cmdline) {
			std::vector<std::string> tokens = TokenizeCommand(cmdline);

			// Loop through tokens
			for (size_t x = 0; x < tokens.size(); x++) {
				if (tokens[x] == "-path") {
					// Make sure there is another token
					if (x + 1 < tokens.size()) {
						// Make sure the directory is valid
						DWORD dwAttributes = GetFileAttributes(tokens[x+1].c_str());
						if (dwAttributes != INVALID_FILE_ATTRIBUTES)
							working_dir = tokens[x+1];
						x++;
					}
				}
#ifdef PHASOR_PC
				else if (tokens[x] == "-mappath") {
					// Make sure there is another token
					if (x + 1 < tokens.size()) {
						// Make sure the directory is valid
						DWORD dwAttributes = GetFileAttributes(tokens[x+1].c_str());
						if (dwAttributes != INVALID_FILE_ATTRIBUTES)
							map_dir = tokens[x+1];
						x++;
					}				
				}
#endif
			}
		}

		// If the path wasn't specified use the default
		if (working_dir.empty()) {
			char szOut[2048] = {0};
			if (SHGetFolderPath(0, CSIDL_MYDOCUMENTS, NULL,
				SHGFP_TYPE_CURRENT, szOut) != S_OK) 
				return false;
			
			working_dir = szOut;
#ifdef PHASOR_PC
			working_dir += "\\My Games\\Halo";
#elif PHASOR_CE
			working_dir += "\\My Games\\Halo CE";
#endif
		}

		// set and force create the directories
		data_dir = working_dir + "\\data";
		script_dir = working_dir + "\\scripts";
		log_dir = working_dir + "\\logs";

		CreateDirectory(data_dir.c_str(), NULL);
		CreateDirectory(script_dir.c_str(), NULL);
		CreateDirectory(log_dir.c_str(), NULL);

		return true;
	}

	std::string GetWorkingDirectory() 
	{
		return working_dir;
	}

	std::string GetMapDirectory()
	{
		return map_dir;
	}

	std::string GetScriptDirectory()
	{
		return script_dir;
	}

	std::string GetDataDirectory()
	{
		return data_dir;
	}

	std::string GetLogDirectory()
	{
		return log_dir;
	}
}