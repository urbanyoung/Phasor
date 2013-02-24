#include "scripttimers.h"
#include "api_readers.h"
#include "../Common/Timers.h"
#include "../Phasor/Globals.h"
#include "../Scripting.h"
#include "../CallHelper.h"

using namespace Common;
using namespace Manager;
using namespace scripting;

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
		if (!still_valid()) return false;

		PhasorCaller caller;
		caller.AddArg(GetID());
		caller.AddArg(++count);
		if (userdata != nullptr) caller.AddArg(userdata->NewCopy());
		return HandleResult<bool>(caller.Call(*state, callback.c_str(), result_bool), false);
	}

};

void l_registertimer(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	DWORD delay = ReadNumber<DWORD>(*args[0]);
	std::string callback = ReadRawString(*args[1]);
	std::unique_ptr<Common::Object> userdata;
	if (args.size() == 3) userdata = std::move(args[2]);

	timer_ptr timer(new ScriptTimer(delay, callback, std::move(userdata), &handler.state));
	DWORD id = g_Timers.AddTimer(std::move(timer));
	AddResultNumber(id, results);
}

void l_removetimer(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	DWORD id = ReadNumber<DWORD>(*args[0]);
	if (!g_Timers.RemoveTimer(id)) handler.RaiseError("removetimer : invalid timer id or a timer tried removing itself in its callback.");
}