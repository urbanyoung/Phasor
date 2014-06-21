#include "scripttimers.h"
#include "api_readers.h"
#include "../../Common/Timers.h"
#include "../../Phasor/Globals.h"
#include "../Scripting.h"
#include "../CallHelper.h"

using namespace Common;
using namespace Manager;
using namespace scripting;

class ScriptTimer;

// The scriptTimers stuff is shit, I don't like this implementation.. 
// Needs a rewrite when Manager is reworked.

// need a rewrite, but for now this shitty solution works
std::map<const Manager::ScriptState*, std::list<DWORD>> scriptTimers;
bool removeScriptTimer(const Manager::ScriptState* script, DWORD id);

class ScriptTimer : public TimerEvent, public CheckedScriptReference
{
	std::string callback;
	std::unique_ptr<Common::Object> userdata;
	DWORD count;

public:
	ScriptTimer(DWORD delay, const std::string& callback, 
		std::unique_ptr<Common::Object> userdata, Manager::ScriptState* state)
		: TimerEvent(delay), CheckedScriptReference(state),
		callback(callback), userdata(std::move(userdata)), count(0)
	{
	}

	// return true to reset the timer
	virtual bool OnExpiration(Timers& timers) override
	{
		if (!still_valid()) {
			// remove all timers for this script (it's invalid)
			auto itr = scriptTimers.find(scriptState);
			if (itr != scriptTimers.end()) scriptTimers.erase(itr);
			return false;
		}

		PhasorScript *phasorState = getPhasorState();
		PhasorCaller caller;
		caller.AddArg(GetID());
		caller.AddArg(++count);
		if (userdata != nullptr) caller.AddArg(userdata->NewCopy());
		bool reset = HandleResult<bool>(caller.Call(*phasorState, callback.c_str(), result_bool), false);
		if (!reset) removeScriptTimer(scriptState, GetID());
		return reset;
	}
};

bool removeScriptTimer(const Manager::ScriptState* script, DWORD id)
{
	auto itr = scriptTimers.find(script);
	if (itr != scriptTimers.end()) {
		auto timersItr = itr->second.begin();
		while (timersItr != itr->second.end()) {
			if (id == *timersItr) {				
				timersItr = itr->second.erase(timersItr);
				return true;
			}
			timersItr++;
		}
	}
	return false;
}

void addScriptTimer(const Manager::ScriptState* script, DWORD id)
{
	std::pair<const Manager::ScriptState*, std::list<DWORD>> 
		p(script, std::list<DWORD>());
	auto result = scriptTimers.insert(p);
	result.first->second.push_back(id);
}

void l_registertimer(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	DWORD delay = ReadNumber<DWORD>(*args[0]);
	std::string callback = ReadRawString(*args[1]);
	std::unique_ptr<Common::Object> userdata;
	if (args.size() == 3) userdata = std::move(args[2]);
	timer_ptr timer(new ScriptTimer(delay, callback, std::move(userdata), &handler.state));
	DWORD id = g_Timers.AddTimer(std::move(timer));
	addScriptTimer(&handler.state, id);

	AddResultNumber(id, results);
}

void l_removetimer(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	DWORD id = ReadNumber<DWORD>(*args[0]);

	// check if it's a timer for this script
	if (!removeScriptTimer(&handler.state, id)) {
		return; // don't raise error for invalid timer ids
	}

	if (!g_Timers.RemoveTimer(id)) handler.RaiseError("removetimer : timer tried removing itself in its callback.");
}