#include "ThreadedLogging.h"

void CThreadedLogging::Initialize()
{
	threadEvent.reset(new CLogThreadEvent(*this, 1000));
	id = thread.InvokeInAux(threadEvent);
	InitializeCriticalSection(&cs);
	AllocateLines();
}

CThreadedLogging::CThreadedLogging(const std::wstring& dir, const std::wstring& file,
	PhasorThread& thread)
	: CLoggingStream(dir, file), thread(thread)
{
	Initialize();
}

CThreadedLogging::CThreadedLogging(const CLoggingStream& stream, PhasorThread& thread)
	: CLoggingStream(stream), thread(thread)
{
	Initialize();
}

CThreadedLogging::~CThreadedLogging()
{
	thread.RemoveAuxEvent(id); // id never null
	DeleteCriticalSection(&cs);
	LogLinesAndCleanup(std::move(lines));
}

void CThreadedLogging::AllocateLines()
{
	lines.reset(new lines_t());
}

void CThreadedLogging::LogLinesAndCleanup(std::unique_ptr<lines_t> data)
{
	// Attempt to write each line
	auto itr = data->begin();
	while (itr != data->end())
	{
		CLoggingStream::Write(*itr);
		itr = data->erase(itr);
	}
}

bool CThreadedLogging::Write(const std::wstring& str)
{
	Lock _(cs);
	lines->push_back(str);
	return true; 
}

CLogThreadEvent::CLogThreadEvent(CThreadedLogging& owner, DWORD dwDelay)
	: owner(owner), PhasorThreadEvent(dwDelay)
{
}

void CLogThreadEvent::OnEventAux(PhasorThread& thread)
{
	// copy the lines to save, then release the lock so the main thread
	// isn't waiting for IO if it needs to log other data
	std::unique_ptr<CThreadedLogging::lines_t> lines;
	{
		Lock _(owner.cs);
		if (owner.lines->size() > 0) {
			lines = std::move(owner.lines);
			owner.AllocateLines();
		}
	} // lock released here

	if (lines != nullptr) {
		printf("Writing\n");
		owner.LogLinesAndCleanup(std::move(lines));
	}
	ReinvokeInAux(thread);
}