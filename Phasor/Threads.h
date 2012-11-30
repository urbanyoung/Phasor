#pragma once

#include "Types.h"

// RAII critical section locking
class Lock
{
private:
	CRITICAL_SECTION& cs;
public:
	Lock(CRITICAL_SECTION& cs) : cs(cs)	{
		EnterCriticalSection(&cs);
	}

	~Lock() {
		LeaveCriticalSection(&cs);
	}
};

class TryLock
{
private:
	CRITICAL_SECTION& cs;
	bool locked;
public:
	TryLock(CRITICAL_SECTION& cs) : cs(cs) {
		locked = TryEnterCriticalSection(&cs) == TRUE;
	}
	bool obtained() const { return locked; }
	~TryLock() {
		if (locked) LeaveCriticalSection(&cs);
	}
};

class Thread
{
protected:

	// The created thread must call this when it's initialized, otherwise
	// it will be terminated.
	void ready()
	{
		success = true;
		SetEvent(h_createEvent);		
	}
	
	// An error occurred during initialization, the thread should terminate.
	void error()
	{
		success = false;
		SetEvent(h_createEvent);
	}

	// Check if the thread should continue running (true - yes, false - no)
	bool check(DWORD dwSleep = 0)
	{
		return WaitForSingleObject(h_syncEvent, dwSleep) != WAIT_OBJECT_0;
	}

private:
	HANDLE h_thread, h_createEvent, h_syncEvent;
	volatile bool success;

	// Called when the thread is created (from that thread)
	static DWORD WINAPI created(LPVOID data)
	{
		Thread* thread = (Thread*)data;
		return thread->thread_main();
	}

public:
	Thread() : h_thread(NULL), h_createEvent(NULL), h_syncEvent(NULL),
		success(false)
	{
	}

	virtual ~Thread()
	{
		CleanupHandle(h_thread);
		CleanupHandle(h_syncEvent);
	}

	void CleanupHandle(HANDLE& handle)
	{
		if (!handle) return;
		CloseHandle(handle);
		handle = 0;
	}

	bool ForceCleanup()
	{
		CleanupHandle(h_createEvent);
		CleanupHandle(h_syncEvent);
		TerminateThread(h_thread, 1);
		CleanupHandle(h_thread);
		return false;
	}

	// Create the thread and call thread_main
	virtual bool run() 
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

		DWORD status = WaitForSingleObject(h_createEvent, 10000);
		CleanupHandle(h_createEvent);

		if (status != WAIT_OBJECT_0)
			return ForceCleanup();
		
		// If the thread failed to initialize, but cleaned up properly
		if (!success) 
			CleanupHandle(h_thread);

		return success;
	}

	// Notify the thread that it should close.
	void close()
	{
		SetEvent(h_syncEvent);
	}

	// Checks if the thread has finished
	bool has_closed()
	{
		if (!h_thread) return true;
		DWORD dwStatus;
		GetExitCodeThread(h_thread, &dwStatus); // assume it worked
		return dwStatus != STILL_ACTIVE;
	}

	// This is the entry point of the thread, gets called after run.
	// This function must call ready once initialized, otherwise it will
	// be aborted.
	// The class implementing this function is always responsible for polling
	// running to check when it should be closed.
	virtual int thread_main() = 0;

	// Example thread_main
	// 	int thread_main()
	// 	{		
	// 		try
	// 		{
	// 			// setup event queues and the like
	// 		}
	// 		catch (...)
	// 		{
	// 			error();
	// 			return 1;
	// 		}
	// 
	// 		ready();
	// 
	// 		while (check())
	// 		{
	// 
	// 		}
	// 
	// 		return 0;
	// 	}
	 
};