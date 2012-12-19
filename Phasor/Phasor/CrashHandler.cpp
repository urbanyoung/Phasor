#include "CrashHandler.h"
#include "../Libraries/StackWalker.h"
#include "../Common/Common.h"
#include "../Common/Types.h"
#include "../Common/MyString.h"
#include "Directory.h"
#include "Halo/Addresses.h"
#include <windows.h>
#include <stdio.h>
#include <DbgHelp.h>

#pragma comment(lib, "DbgHelp.lib")

namespace CrashHandler
{
	using namespace Common;

	class MyStackWalker : public StackWalker
	{
	public:
		MyStackWalker() : StackWalker() {}
		MyStackWalker(DWORD dwProcessId, HANDLE hProcess) : StackWalker(dwProcessId, hProcess) {}
		virtual void OnOutput(LPCSTR szText) { 
			printf(szText); 
			StackWalker::OnOutput(szText);
		}
	};

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

	LONG WINAPI OnUnhandledException(PEXCEPTION_POINTERS pExceptionInfo)
	{
		if (!bPassOn) // exception occurred while closing, force kill the server
			ExitProcess(1);

		bPassOn = false;

		MINIDUMP_EXCEPTION_INFORMATION ei;
		ei.ExceptionPointers = pExceptionInfo;
		ei.ThreadId = GetCurrentThreadId();
		ei.ClientPointers = FALSE;

		DWORD dwProcessId = GetCurrentProcessId();

		SYSTEMTIME stLocalTime;
		GetLocalTime(&stLocalTime);
		wchar_t CrashDumpW[1024];
		swprintf_s(CrashDumpW, NELEMS(CrashDumpW), 
				L"%s\\%s-%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp", 
				g_CrashDirectory.c_str(), L"Phasor", L"Pre-release", 
				stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay, 
				stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond, 
				dwProcessId, ei.ThreadId);
		// CreateFileW can write into CrashDumpW so can't be const memory
		HANDLE hFile = CreateFileW(CrashDumpW, GENERIC_READ | GENERIC_WRITE,
			NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		MiniDumpWriteDump(GetCurrentProcess(), dwProcessId, hFile, 
			MiniDumpScanMemory, &ei, NULL, NULL);
		CloseHandle(hFile);
		// this might be better suited for halo crashes
		/*MyStackWalker walker(GetCurrentProcessId(), GetCurrentProcess());
		walker.ShowCallstack(GetCurrentThread(), pExceptionInfo->ContextRecord);*/

		return EXCEPTION_EXECUTE_HANDLER;
	}
}