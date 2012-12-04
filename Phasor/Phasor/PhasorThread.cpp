#include "PhasorThread.h"

#define EVENT_ID(x) (DWORD)&*x

// ------------------------------------------------------------------------
PhasorThreadEvent::PhasorThreadEvent(DWORD dwDelay) : dwDelay(dwDelay)
{
	SetExpiry(dwDelay);
}

PhasorThreadEvent::~PhasorThreadEvent() 
{
}

void PhasorThreadEvent::SetExpiry(DWORD dwDelay)
{
	dwExpiryTime = dwDelay > 0 ? GetTickCount() + dwDelay : 0;
}

// Reinvoke the event 
void PhasorThreadEvent::Reinvoke(DWORD dwDelay, 
	PhasorThread& thread, event_dest_t dest)
{
	if (dwDelay != 0) this->dwDelay = dwDelay;
	SetExpiry(this->dwDelay);
	thread.SetReinvoke(dest);
}

// The event wants to be reinvoked in main
void PhasorThreadEvent::ReinvokeInMain(PhasorThread& thread, DWORD dwDelay)
{
	Reinvoke(dwDelay, thread, DEST_MAIN);		
}

// The event wants to be reinvoked in aux
void PhasorThreadEvent::ReinvokeInAux(PhasorThread& thread, DWORD dwDelay)
{
	Reinvoke(dwDelay, thread, DEST_AUX);
}	

DWORD PhasorThreadEvent::GetExpiryTime() const
{ 
	return dwExpiryTime;
}

bool PhasorThreadEvent::ready(DWORD dwCurTicks) const 
{
	return dwExpiryTime <= dwCurTicks; 
}

// ------------------------------------------------------------------------
PhasorThread::PhasorThread() : Thread(), 
	dwPhasorThreadId(NULL), dwMainThreadId(NULL), reinvoke_in(DEST_NONE)	
{
	InitializeCriticalSection(&cs);
}

PhasorThread::~PhasorThread() 
{
	DeleteCriticalSection(&cs);
}

// The event currently being processed wants to be reinvoked
void PhasorThread::SetReinvoke(event_dest_t dest)
{
	reinvoke_in = dest;
}

// Adds an event to the specified list
DWORD PhasorThread::AddEvent(event_dest_t dest, std::shared_ptr<PhasorThreadEvent>& e)
{
	Lock _(cs);
	eventlist_t* eventList = NULL;
	if (dest == DEST_MAIN) eventList = &mainEvents;
	else eventList = &auxEvents;

	DWORD id = EVENT_ID(e);
	// was going to keep list sorted based on expiry time, but no
	// point.. will never have many events at once. plus most won't
	// have an expiry time
	eventList->push_back(e);	

	return id;
}		

// Processes any required events in the specified thread. A lock for
// the lists should have been obtained before calling.
void PhasorThread::ProcessEventsLocked(event_dest_t dest, bool ignoretime)
{
	eventlist_t* eventList = NULL, *other = NULL;
	if (dest == DEST_MAIN) {
		eventList = &mainEvents;
		other = &auxEvents;
	} else {
		eventList = &auxEvents;
		other = &mainEvents;
	}

	DWORD curTime = GetTickCount();

	// for loop so we don't reprocess events
	auto itr = eventList->begin();
	size_t size = eventList->size();
	for (size_t i = 0; i < size; i++) {
		PhasorThreadEvent& e = **itr;
		if (ignoretime || e.ready(curTime)) {
			reinvoke_in = DEST_NONE;
			if (dest == DEST_MAIN)
				e.OnEventMain(*this);
			else
				e.OnEventAux(*this);
			// check if this event is to be reinvoked
			if (reinvoke_in == dest)
				eventList->push_back(std::move(*itr));
			else if (reinvoke_in != DEST_NONE)
				other->push_back(std::move(*itr)); 

			itr = eventList->erase(itr);
			continue;			
		} 
		itr++;
	}
}

// This will only be called from the main thread
bool PhasorThread::run()
{
	auxEvents.clear();
	mainEvents.clear();
	dwMainThreadId = GetCurrentThreadId();
	return Thread::run();
}

// Process events
void PhasorThread::ProcessEvents(bool block, bool main, bool ignoretime)
{
	event_dest_t dest = main ? DEST_MAIN : DEST_AUX;
	if (!block) { // don't want halo thread to block
		TryLock lock(cs);
		if (lock.obtained()) ProcessEventsLocked(dest, ignoretime);
	} else {
		Lock _(cs);
		ProcessEventsLocked(dest, ignoretime);
	}
}

DWORD PhasorThread::InvokeInMain(std::shared_ptr<PhasorThreadEvent> e)
{
	return AddEvent(DEST_MAIN, e);
}

DWORD PhasorThread::InvokeInAux(std::shared_ptr<PhasorThreadEvent> e)
{
	return AddEvent(DEST_AUX, e);
}

void PhasorThread::RemoveAuxEvent(DWORD id)
{
	Lock _(cs);
	auto itr = auxEvents.begin();
	while (itr != auxEvents.end())
	{
		if (EVENT_ID(*itr) == id) {
			itr = auxEvents.erase(itr);
			//break; // remove all events with this id
		} else itr++;
	}
}

// Thread entry point
int PhasorThread::thread_main()
{		
	try
	{
		dwPhasorThreadId = GetCurrentThreadId();
		// setup event queues and the like
	}
	catch (...)
	{
		error();
		return 1;
	}

	ready();

	while (check(15)) // will sleep(15) too (effectively)
	{
		ProcessEvents(true, false);
	}
	ProcessEvents(true, false, true); // make sure all events are done

	return 0;
}


