#include "Timers.h"

// ------------------------------------------------------------------------
//
TimerEvent::TimerEvent(DWORD dwDelay) : dwDelay(dwDelay)
{
	SetExpiry(dwDelay);
}

TimerEvent::~TimerEvent() 
{
}

void TimerEvent::SetExpiry(DWORD dwDelay)
{
	dwExpiryTime = GetTickCount() + dwDelay;
}

// Get a timer's id
DWORD TimerEvent::GetID() const
{
	return (DWORD)this;
}

bool TimerEvent::ready(DWORD dwCurTicks) const
{
	return dwCurTicks >= dwExpiryTime;
}

void TimerEvent::Reset()
{
	SetExpiry(dwDelay);
}

// ------------------------------------------------------------------------
//
Timers::Timers() : currentId(NULL)
{
}

Timers::~Timers()
{
}

DWORD Timers::AddTimer(std::unique_ptr<TimerEvent> e)
{
	timerlist.push_back(std::move(e));
	return (DWORD)timerlist.back().get();
}

void Timers::RemoveTimer(DWORD id)
{
	if (id == currentId) return;
	auto itr = timerlist.begin();
	while (itr != timerlist.end()) {
		if ((*itr)->GetID() == id) {
			itr = timerlist.erase(itr);
			break;
		}
		itr++;
	}
}

void Timers::Process()
{
	DWORD curTime = GetTickCount();

	auto itr = timerlist.begin();
	while (itr != timerlist.end()) {
		TimerEvent& timer = **itr;
		if (timer.ready(curTime)) {
			currentId = timer.GetID();
			if (timer.OnExpiration(*this))
				timer.Reset();
			else {
				itr = timerlist.erase(itr);
				continue;
			}
		}
		itr++;
	}

	currentId = NULL;
}
