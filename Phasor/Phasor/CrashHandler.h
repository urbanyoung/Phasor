#pragma once

#include <windows.h>
#include <string>

namespace CrashHandler
{
	struct s_exception_handler_info
	{
		HANDLE hProcess;
		HANDLE hThread;
		DWORD dwProcessId;
		DWORD dwThreadId;
		PEXCEPTION_POINTERS pExceptionInfo;
	};

	// This installs the exception catches (globally and through hooking Halo's).
	void InstallCatchers();

	void WriteCrashMiniDump(std::wstring& out_dir, 
		s_exception_handler_info& info);
}