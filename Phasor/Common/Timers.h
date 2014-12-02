#pragma once

#include <memory>
#include <list>
#include "Types.h"

class Timers;

class TimerEvent
{
private:
	DWORD dwExpiryTime, dwDelay;
	void SetExpiry(DWORD dwDelay);

protected:
	TimerEvent(DWORD dwDelay);

public:
	virtual ~TimerEvent();

	// Get a timer's id
	DWORD GetID() const;

	// used to check if the timer is ready
	bool ready(DWORD dwCurTicks) const;

	// Reset the timer
	void Reset();

	// return true to reset the timer
	virtual bool OnExpiration(Timers& timers) = 0;
};

typedef std::unique_ptr<TimerEvent> timer_ptr;

class Timers
{
private:
	typedef std::list<timer_ptr> timerlist_t;
	timerlist_t timerlist;
	DWORD currentId;

public:
	Timers();
	virtual ~Timers();
	
	// Should be called periodically to process complete timers
	void Process();

	// Adds a new timer
	DWORD AddTimer(timer_ptr e);

	// Removes a timer
	bool RemoveTimer(DWORD id);
	void RemoveAllTimers();
};