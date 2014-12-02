#pragma once

#include "Types.h"

class Lock
{
private:
	CRITICAL_SECTION& cs;
public:
	Lock(CRITICAL_SECTION& cs);
	~Lock();
};

class TryLock
{
private:
	CRITICAL_SECTION& cs;
	bool locked;
public:
	TryLock(CRITICAL_SECTION& cs);
	bool obtained() const;
	~TryLock();
};

class Thread
{
protected:

	// The created thread must call this when it's initialized, otherwise
	// it will be terminated.
	void ready();
	
	// An error occurred during initialization, the thread should terminate.
	void error();

	// Check if the thread should continue running (true - yes, false - no)
	bool check(DWORD dwSleep = 0);

private:
	HANDLE h_thread, h_createEvent, h_syncEvent;
	volatile bool success;

	// Called when the thread is created (from that thread)
	static DWORD WINAPI created(LPVOID data);

	void CleanupHandle(HANDLE& handle);
	bool ForceCleanup();

public:
	Thread();
	virtual ~Thread();	

	// Create the thread and call thread_main
	virtual bool run();

	// Notify the thread that it should close.
	void close();

	// Checks if the thread has finished
	bool has_closed();

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