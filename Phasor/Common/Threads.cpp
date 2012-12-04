#pragma once

#include "Threads.h"

Lock::Lock(CRITICAL_SECTION& cs) : cs(cs)
{
	EnterCriticalSection(&cs);
}

Lock::~Lock() 
{
	LeaveCriticalSection(&cs);
}


// ------------------------------------------------------------------------

TryLock::TryLock(CRITICAL_SECTION& cs) : cs(cs)
{
	locked = TryEnterCriticalSection(&cs) == TRUE;
}

bool TryLock::obtained() const 
{ 
	return locked; 
}

TryLock::~TryLock() 
{
	if (locked) LeaveCriticalSection(&cs);
}

// ------------------------------------------------------------------------

Thread::Thread() : h_thread(NULL), h_createEvent(NULL), h_syncEvent(NULL),
	success(false)
{
}

Thread::~Thread()
{
	CleanupHandle(h_thread);
	CleanupHandle(h_syncEvent);
}

void Thread::ready()
{
	success = true;
	SetEvent(h_createEvent);		
}

void Thread::error()
{
	success = false;
	SetEvent(h_createEvent);
}

bool Thread::check(DWORD dwSleep)
{
	return WaitForSingleObject(h_syncEvent, dwSleep) != WAIT_OBJECT_0;
}

DWORD WINAPI Thread::created(LPVOID data)
{
	Thread* thread = (Thread*)data;
	return thread->thread_main();
}

void Thread::CleanupHandle(HANDLE& handle)
{
	if (!handle) return;
	CloseHandle(handle);
	handle = 0;
}

bool Thread::ForceCleanup()
{
	CleanupHandle(h_createEvent);
	CleanupHandle(h_syncEvent);
	TerminateThread(h_thread, 1);
	CleanupHandle(h_thread);
	return false;
}

bool Thread::run() 
{
	success = false;

	h_thread = CreateThread(NULL, 0, created, (LPVOID)this, 
		CREATE_SUSPENDED, NULL);
	if (h_thread == NULL) return false;

	h_createEvent = CreateEvent(0, TRUE, FALSE, NULL);
	h_syncEvent = CreateEvent(0, TRUE, FALSE, NULL);

	if (h_createEvent == NULL || h_syncEvent == NULL) 
		return ForceCleanup();			

	if (ResumeThread(h_thread) == -1) 
		return ForceCleanup();

	DWORD status = WaitForSingleObject(h_createEvent, 5000);
	CleanupHandle(h_createEvent);

	if (status != WAIT_OBJECT_0)
		return ForceCleanup();

	// If the thread failed to initialize, but cleaned up properly
	if (!success) 
		CleanupHandle(h_thread);

	return success;
}

void Thread::close()
{
	SetEvent(h_syncEvent);
}

bool Thread::has_closed()
{
	if (!h_thread) return true;
	DWORD dwStatus;
	GetExitCodeThread(h_thread, &dwStatus); // assume it worked
	return dwStatus != STILL_ACTIVE;
}
