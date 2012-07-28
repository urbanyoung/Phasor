#include "Phasor.h"
#include <vector>
#include <windows.h>
#include <Shlobj.h>

namespace Phasor
{
	Error::Error()
	{
		hasErr = false;
	}

	Error::~Error()
	{

	}

	void Error::SetError(const std::string& error)
	{
		printf("Error: %s\n", error.c_str());
		this->err = error;
		hasErr = true;
	}

	std::string Error::GetError() const
	{
		return err;
	}

	bool Error::hasError() const
	{
		return hasErr;
	}

	// --------------------------------------------------------------------
	// 
	// Directory stuff
	std::string working_dir, map_dir, data_dir, script_dir;
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

		CreateDirectory(data_dir.c_str(), NULL);
		CreateDirectory(script_dir.c_str(), NULL);

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
}