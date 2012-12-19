#include <windows.h>
#include <stdio.h>
#include "../Phasor/Phasor/CrashHandler.h"
#include "../Phasor/Common/MyString.h"

int main(int argc, char** args)
{
	if (argc != 2) return 1;
	std::wstring out_dir = GetCommandLineW();
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE); 

	CrashHandler::s_exception_handler_info info;
	DWORD total_read = 0;
	while (total_read < sizeof(info)) {
		BYTE* pRead = (BYTE*)&info + total_read;
		DWORD dwRead = 0;
		BOOL bSuccess = ReadFile(hStdin, pRead, sizeof(info) - total_read, &dwRead, NULL);
		if(!bSuccess || dwRead == 0 ) break; 
		total_read += dwRead;
		printf("reading\n");
	}

	if (total_read != sizeof(info)) return 1;
	printf("%08X %08X\n", info.pExceptionInfo, info.dwThreadId);
	CrashHandler::WriteCrashMiniDump(out_dir, info);

	return 0;
}
