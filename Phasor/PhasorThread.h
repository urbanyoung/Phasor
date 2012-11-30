// PhasorThread.h
#pragma once

#include "Threads.h"
#include <memory>
#include <stdio.h>
#include <list>

class PhasorThread;

enum event_dest_t
{
	DEST_NONE = 0,
	DEST_MAIN,
	DEST_AUX
};

class PhasorThreadEvent
{
private:
	DWORD dwExpiryTime;

	void SetExpiry(DWORD dwDelay);

protected:
	PhasorThreadEvent(DWORD dwDelay);

	void Reinvoke(DWORD dwDelay, PhasorThread& thread, event_dest_t dest);

	// Reinvoke the event in the specified thread (next check)
	void ReinvokeInMain(PhasorThread& thread, DWORD dwDelay = 0);
	void ReinvokeInAux(PhasorThread& thread, DWORD dwDelay = 0);

public:
	//virtual std::unique_ptr<PhasorThreadEvent> Create(DWORD dwDelayMilliseconds) = 0;
	virtual ~PhasorThreadEvent();

	DWORD GetExpiryTime() const;
	bool ready(DWORD dwCurTicks) const;
	
	// Event is invoked in the aux thread
	virtual void OnEventAux(PhasorThread& thread) = 0;

	// Event is invoked in the main thread
	virtual void OnEventMain(PhasorThread& thread) = 0;
};

// This class implements Phasor's worker (auxillary) thread.
class PhasorThread : public Thread
{
private:
	DWORD dwPhasorThreadId, dwMainThreadId;
	typedef std::list<std::unique_ptr<PhasorThreadEvent>> eventlist_t;
	eventlist_t auxEvents, mainEvents;
	CRITICAL_SECTION cs;

	// This variable can be set by events if they want to be raised again.
	event_dest_t reinvoke_in;

	// The event currently being processed wants to be reinvoked
	void SetReinvoke(event_dest_t dest);
	
	// Adds an event to the specified list
	void AddEvent(event_dest_t dest, std::unique_ptr<PhasorThreadEvent>& e);

	// Processes any required events in the specified thread. A lock for
	// the lists should have been obtained before calling.
	void ProcessEventsLocked(event_dest_t dest);

public:
	PhasorThread();
	virtual ~PhasorThread();

	// This will only be called from the main thread
	virtual bool run();

	// Process events
	void ProcessEvents(bool block=false, bool main=true);

	void InvokeInMain(std::unique_ptr<PhasorThreadEvent> e);
	void InvokeInAux(std::unique_ptr<PhasorThreadEvent> e);

	// Thread entry point
	int thread_main();

	friend class PhasorThreadEvent;
};

class TestEvent : public PhasorThreadEvent
{
private:
	int count;
	TestEvent(DWORD dwDelayMilliseconds) : 
	   PhasorThreadEvent(dwDelayMilliseconds)
	{
		count = 0;
	}

public:
	static std::unique_ptr<PhasorThreadEvent> Create(DWORD dwDelayMilliseconds) {
		return std::unique_ptr<PhasorThreadEvent>(
			new TestEvent(dwDelayMilliseconds));
	}
	virtual ~TestEvent() {}

	// Event is invoked in the aux thread
	void OnEventAux(PhasorThread& thread) {
		printf("Invoked in auxillary %08X\n", GetCurrentThreadId());
		ReinvokeInMain(thread);
		count++;
	}

	// Event is invoked in the main thread
	void OnEventMain(PhasorThread& thread) {
		printf("Invoked in main %08X\n", GetCurrentThreadId());
		//if (count < 1)
		ReinvokeInAux(thread, 10);

	}
};