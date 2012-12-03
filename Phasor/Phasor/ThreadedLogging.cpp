#include "ThreadedLogging.h"

void CThreadedLogging::Initialize()
{
	InitializeCriticalSection(&cs);
	id = NULL;
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
	if (id) thread.RemoveAuxEvent(id);
	DeleteCriticalSection(&cs);
	LogLinesLocked();
}

void CThreadedLogging::LogLinesLocked()
{
	// Attempt to write each line
	auto itr = lines.begin();
	while (itr != lines.end())
	{
		CLoggingStream::Write(*itr);
		itr = lines.erase(itr);
	}
}

bool CThreadedLogging::Write(const std::wstring& str)
{
	Lock _(cs);
	lines.push_back(str);

	if (id == NULL) {		
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
	Lock _(owner.cs);
	owner.LogLinesLocked();
	owner.id = NULL;
}