#include "CrashHandler.h"
//#include "../Libraries/StackWalker.h"
#include "../Common/MyString.h"
#ifndef PHASOR_CRASH_HANDLER
#include "../Common/Common.h"
#include "Halo/Addresses.h"
#include "Directory.h"
#endif
#include <windows.h>
#include <stdio.h>
#include <DbgHelp.h>

#pragma comment(lib, "DbgHelp.lib")

namespace CrashHandler
{
	/*class MyStackWalker : public StackWalker
	{
	public:
		MyStackWalker() : StackWalker() {}
		MyStackWalker(DWORD dwProcessId, HANDLE hProcess) : StackWalker(dwProcessId, hProcess) {}
		virtual void OnOutput(LPCSTR szText) { 
			printf(szText); 
			StackWalker::OnOutput(szText);
		}
	};*/

#ifndef PHASOR_CRASH_HANDLER
	using namespace Common;

	// Handler for all unhandled exceptions
	LONG WINAPI OnUnhandledException(PEXCEPTION_POINTERS pExceptionInfo);
	bool bPassOn = true;

	DWORD halo_seh_ret = 0;
	__declspec(naked) void HaloExceptionHandler()
	{
		__asm
		{
			pop halo_seh_ret

			pushad
			push 0
			call OnUnhandledException
			popad

			PUSH EBP
			MOV EBP,ESP
			SUB ESP,0x8

			push halo_seh_ret
			ret
		}
	}

	// This installs the exception catches (globally and through hooking Halo's).
	void InstallCatchers()
	{
		SetUnhandledExceptionFilter(OnUnhandledException);

		// Hook Halo's exception handler
		CreateCodeCave(CC_EXCEPTION_HANDLER, 6, HaloExceptionHandler);
	}

	BOOL MyCreatePipe(HANDLE &read, HANDLE &write)
	{
		SECURITY_ATTRIBUTES pipesa;
		pipesa.bInheritHandle = TRUE;
		pipesa.lpSecurityDescriptor = NULL;
		pipesa.nLength = sizeof(SECURITY_ATTRIBUTES);
		if (!CreatePipe(&read, &write, &pipesa, 0)) return FALSE;
		return TRUE;
	}

	HANDLE MakeDuplicateHandle(HANDLE hTargetProcess, HANDLE hSrcHandle, 
		BOOL& success)
	{
		HANDLE hOutHandle = NULL;
		success &= DuplicateHandle(GetCurrentProcess(), hSrcHandle, hTargetProcess,
			&hOutHandle, DUPLICATE_SAME_ACCESS, TRUE, 0);
		return hOutHandle;
	}

	void WriteCrashMiniDumpFromHalo(s_exception_handler_info& einfo)
	{
		einfo.hProcess = GetCurrentProcess();
		einfo.hThread = GetCurrentThread();
		WriteCrashMiniDump(g_CrashDirectory, einfo);
	}

	void CleanupHandles(HANDLE h1, HANDLE h2)
	{
		if (h1 != NULL) CloseHandle(h1);
		if (h2 != NULL) CloseHandle(h2);
	}

	LONG WINAPI OnUnhandledException(PEXCEPTION_POINTERS pExceptionInfo)
	{
		if (!bPassOn) // exception occurred while closing, force kill the server
			ExitProcess(1);

		bPassOn = false;
		BOOL success = TRUE;

		HANDLE hChildStdInRd = NULL, hChildStdInWr = NULL;
		success &= MyCreatePipe(hChildStdInRd, hChildStdInWr);
		success &= SetHandleInformation(hChildStdInWr, HANDLE_FLAG_INHERIT, 0);

		s_exception_handler_info einfo;
		einfo.dwProcessId = GetCurrentProcessId();
		einfo.dwThreadId = GetCurrentThreadId();
		einfo.pExceptionInfo = pExceptionInfo;
		printf("Exception ptr %08X\n", pExceptionInfo);

		// try logging from this process
		if (!success) {
			CleanupHandles(hChildStdInRd, hChildStdInWr);
			WriteCrashMiniDumpFromHalo(einfo);
			return EXCEPTION_EXECUTE_HANDLER;
		}

		STARTUPINFO si = {0};
		si.cb = sizeof(STARTUPINFO);
		si.hStdError = GetStdHandle(STD_OUTPUT_HANDLE);
		si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		si.hStdInput = hChildStdInRd;
		si.dwFlags = STARTF_USESTDHANDLES;

		PROCESS_INFORMATION pi = {0};
		std::string out_dir = NarrowString(g_CrashDirectory);
		success = CreateProcess("D:\\Development\\C++\\Phasor\\Debug\\phasor_crash_handler.exe", 
			(LPSTR)out_dir.c_str(), NULL, NULL, 
			TRUE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);	

		if (success) {
			einfo.hProcess = MakeDuplicateHandle(pi.hProcess, GetCurrentProcess(), success);
			einfo.hThread = MakeDuplicateHandle(pi.hProcess, GetCurrentThread(), success);

			if (success) {
				DWORD dwWritten = 0;
				while (success && dwWritten < sizeof(einfo)) {
					BYTE* pData = (BYTE*)&einfo + dwWritten;
					success &= WriteFile(hChildStdInWr, pData, sizeof(einfo)-dwWritten, 
						&dwWritten, NULL);
					if (dwWritten == 0) success = FALSE;
				}
			} 
		}		

		if (success) {
			ResumeThread(pi.hThread);
			WaitForSingleObject(pi.hProcess, 5000);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			system("PAUSE");
		} else {			
			WriteCrashMiniDumpFromHalo(einfo);
		}
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CleanupHandles(hChildStdInRd, hChildStdInWr);

		return EXCEPTION_EXECUTE_HANDLER;
	}
#endif

	void WriteCrashMiniDump(std::wstring& out_dir, 
		s_exception_handler_info& info)
	{
		SYSTEMTIME stLocalTime;
		GetLocalTime(&stLocalTime);

		std::wstring out_file = m_swprintf(L"%s\\%s-%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp", 
			out_dir.c_str(), L"Phasor", L"Pre-release", 
			stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay, 
			stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond, 
			info.dwProcessId, info.dwThreadId);

		MINIDUMP_EXCEPTION_INFORMATION ei;
		ei.ClientPointers = FALSE;
		ei.ExceptionPointers = info.pExceptionInfo;
		ei.ThreadId = info.dwThreadId;

		HANDLE hFile = CreateFileW(out_file.c_str(), GENERIC_READ | GENERIC_WRITE,
			NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		MiniDumpWriteDump(info.hProcess, info.dwProcessId, hFile, 
			MiniDumpScanMemory, &ei, NULL, NULL);
		CloseHandle(hFile);
	}
}