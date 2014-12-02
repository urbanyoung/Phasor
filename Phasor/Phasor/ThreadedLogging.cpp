#include "ThreadedLogging.h"

void CThreadedLogging::Initialize(DWORD dwDelay)
{
	InitializeCriticalSection(&cs);
	InitializeCriticalSection(&loggingStreamCS);
	CLoggingStream::EnableTimestamp(false); // we'll deal with it
	bDoTimestamp = true; // timestamp by default
	threadEvent.reset(new CLogThreadEvent(*this, dwDelay));
	id = thread.InvokeInAux(threadEvent);
	AllocateLines();
	this->dwDelay = dwDelay;
}

CThreadedLogging::CThreadedLogging(const std::wstring& dir, const std::wstring& file,
	const std::wstring& movedir,
	PhasorThread& thread, DWORD dwDelay)
	: CLoggingStream(dir, file, movedir), thread(thread)
{
	Initialize(dwDelay);
}

CThreadedLogging::CThreadedLogging(const CLoggingStream& stream, PhasorThread& thread,
	DWORD dwDelay)
	: CLoggingStream(stream), thread(thread)
{
	Initialize(dwDelay);
}

std::unique_ptr<COutStream> CThreadedLogging::clone() const
{
	return std::unique_ptr<COutStream>(new CThreadedLogging
		(fileDirectory,fileName,moveDirectory, thread, dwDelay));
}

CThreadedLogging::~CThreadedLogging()
{
	thread.RemoveAuxEvent(id); // id never null
	LogLinesAndCleanup(std::move(lines));
	DeleteCriticalSection(&cs);
	DeleteCriticalSection(&loggingStreamCS);
}

void CThreadedLogging::AllocateLines()
{
	lines.reset(new lines_t());
}

void CThreadedLogging::LogLinesAndCleanup(std::unique_ptr<lines_t> data)
{
	Lock _(loggingStreamCS);
	size_t size = data->size();
	if (size == 0) return;
	if (size == 1) { // no point copying for one entry
		CLoggingStream::Write(*data->begin());
		data->clear();
		return;
	}

	std::wstring out;
	out.reserve(1 << 15); // 32kb should always be enough for our purposes
	// increasing this can improve performance tho (if there's a lot of data
	// to write at a time)
	
	// todo: handle write failures
	auto itr = data->begin();
	while (itr != data->end()) {
		// don't want to need to expand the buffer
		if (out.size() + itr->size() > out.capacity()) {
			CLoggingStream::Write(out);
			out.clear();
			if (itr->size() < out.capacity()) out += *itr;
			else CLoggingStream::Write(*itr);
		} else out += *itr;
		itr++;// = data->erase(itr);
	}
	CLoggingStream::Write(out);
	data->clear();
}

bool CThreadedLogging::Write(const std::wstring& str)
{
	Lock _(cs);
	if (bDoTimestamp)
		lines->push_back(PrependTimestamp(str));
	else
		lines->push_back(str);
	return true; 
}

void CThreadedLogging::SetMoveSize(DWORD kbSize)
{
	Lock _(loggingStreamCS);
	CLoggingStream::SetMoveSize(kbSize);
}

void CThreadedLogging::SetMoveDirectory(const std::wstring& move_to)
{
	Lock _(loggingStreamCS);
	CLoggingStream::SetMoveDirectory(move_to);
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
	bDoTimestamp = state;
	CLoggingStream::EnableTimestamp(state);
}

void CThreadedLogging::AppendData(const std::wstring& str)
{
	Lock _(loggingStreamCS);
	COutStream::AppendData(str);
}

void CThreadedLogging::AppendData(wchar_t c)
{
	Lock _(loggingStreamCS);
	COutStream::AppendData(c);
}

void CThreadedLogging::Reserve(size_t size)
{
	Lock _(loggingStreamCS);
	COutStream::Reserve(size);
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

	if (lines != nullptr) owner.LogLinesAndCleanup(std::move(lines));
	
	ReinvokeInAux(thread);
}