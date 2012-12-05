#include "ThreadedLogging.h"

void CThreadedLogging::Initialize(DWORD dwDelay)
{
	InitializeCriticalSection(&cs);
	InitializeCriticalSection(&loggingStreamCS);
	threadEvent.reset(new CLogThreadEvent(*this, dwDelay));
	id = thread.InvokeInAux(threadEvent);
	AllocateLines();
}

CThreadedLogging::CThreadedLogging(const std::wstring& dir, const std::wstring& file,
	PhasorThread& thread, DWORD dwDelay)
	: CLoggingStream(dir, file), thread(thread)
{
	Initialize(dwDelay);
}

CThreadedLogging::CThreadedLogging(const CLoggingStream& stream, PhasorThread& thread,
	DWORD dwDelay)
	: CLoggingStream(stream), thread(thread)
{
	Initialize(dwDelay);
}

CThreadedLogging::~CThreadedLogging()
{
	thread.RemoveAuxEvent(id); // id never null
	DeleteCriticalSection(&cs);
	LogLinesAndCleanup(std::move(lines));
	DeleteCriticalSection(&loggingStreamCS);
}

void CThreadedLogging::AllocateLines()
{
	lines.reset(new lines_t());
}

void CThreadedLogging::LogLinesAndCleanup(std::unique_ptr<lines_t> data)
{
	Lock _(loggingStreamCS);
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

void CThreadedLogging::SetMoveInfo(const std::wstring& move_to, DWORD kbSize)
{
	Lock _(loggingStreamCS);
	CLoggingStream::SetMoveInfo(move_to, kbSize);
}

void CThreadedLogging::SetOutFile(const std::wstring& directory,const std::wstring& fileName)
{
	Lock _(loggingStreamCS);
	CLoggingStream::SetOutFile(directory, fileName);
}

void CThreadedLogging::SetOutFile(const std::wstring& fileName)
{
	Lock _(loggingStreamCS);
	CLoggingStream::SetOutFile(fileName);
}

void CThreadedLogging::EnableTimestamp(bool state)
{
	Lock _(loggingStreamCS);
	CLoggingStream::EnableTimestamp(state);
}

std::wstring CThreadedLogging::PrependTimestamp(const std::wstring& str)
{
//	Lock _(loggingStreamCS);
	return CLoggingStream::PrependTimestamp(str);
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