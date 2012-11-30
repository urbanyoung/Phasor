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

typedef std::unique_ptr<TimerEvent> timer_t;

class Timers
{
private:
	typedef std::list<timer_t> timerlist_t;
	timerlist_t timerlist;
	DWORD currentId;

public:
	Timers();
	virtual ~Timers();

	// Should be called periodically to process complete timers
	void Process();

	// Adds a new timer
	DWORD AddTimer(timer_t e);

	// Removes a timer
	void RemoveTimer(DWORD id);
};


class TestTimer : public TimerEvent
{
	TestTimer(DWORD dwDelay) : TimerEvent(dwDelay)
	{
	}
public:

	static std::unique_ptr<TestTimer> Create(DWORD dwDelay)
	{
		return std::unique_ptr<TestTimer>(new TestTimer(dwDelay));
	}
	
	bool OnExpiration(Timers& timers)
	{
		printf("Timer expired.. Resetting\n");
		return true;
	}
};

class TestTimer1 : public TimerEvent
{
	DWORD id;
	TestTimer1(DWORD dwDelay, DWORD id) : TimerEvent(dwDelay),id(id)
	{
	}
public:
	static std::unique_ptr<TestTimer1> Create(DWORD dwDelay, DWORD id)
	{
		return std::unique_ptr<TestTimer1>(new TestTimer1(dwDelay,id));
	}

	bool OnExpiration(Timers& timers)
	{
		printf("timer1.. removing 0\n");
		timers.RemoveTimer(id);
		return true;

	}
};