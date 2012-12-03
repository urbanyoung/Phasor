#include "ThreadedLogging.h"

void CThreadedLogging::Initialize()
{
	InitializeCriticalSection(&cs);
	id = NULL;
	AllocateLines();
}

CThreadedLogging::CThreadedLogging(const std::wstring& file, PhasorThread& thread)
	: CLoggingStream(file), thread(thread)
{
	Initialize();
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
	if (id != NULL)
	{
		thread.RemoveAuxEvent(id);
		id = NULL;
		// Lock _(cs); not needed
	}
	
	DeleteCriticalSection(&cs);
	LogLinesAndCleanup(lines);
}

void CThreadedLogging::AllocateLines()
{
	lines = new lines_t();
}

void CThreadedLogging::LogLinesAndCleanup(lines_t* data)
{
	// Attempt to write each line
	auto itr = data->begin();
	while (itr != data->end())
	{
		CLoggingStream::Write(*itr);
		itr = data->erase(itr);
	}
	delete data;
}

bool CThreadedLogging::Write(const std::wstring& str)
{
	Lock _(cs);
	lines->push_back(str);

	if (id == NULL) { // no event running, so can't deadlock
		id = thread.InvokeInAux(std::unique_ptr<PhasorThreadEvent>
			(new CLogThreadEvent(*this)));
	}
	return true; 
}

CLogThreadEvent::CLogThreadEvent(CThreadedLogging& owner) : owner(owner),
	PhasorThreadEvent(0)
{
}

void CLogThreadEvent::OnEventAux(PhasorThread&)
{
	// copy the lines to save, then release the lock so the main thread
	// isn't waiting for IO if it needs to log other data
	CThreadedLogging::lines_t* lines;
	{
		Lock _(owner.cs);
		lines = owner.lines;
		owner.AllocateLines();
		owner.id = NULL;
	} // lock released here
	
	owner.LogLinesAndCleanup(lines);
}