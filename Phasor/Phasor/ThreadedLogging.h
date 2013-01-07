// ThreadedLogging.h
// Provides an interface for logging messages to disk from another thread.
// Phasor uses this to reduce lag caused by logging messages (disk io is 
// really slow).
#pragma once

#include "Logging.h"
#include "PhasorThread.h"

class CLogThreadEvent;

#define DEFAULT_SAVE_DELAY	1000

class CThreadedLogging : public CLoggingStream
{
private:
	CRITICAL_SECTION cs, loggingStreamCS;
	DWORD id;
	PhasorThread& thread;
	std::shared_ptr<CLogThreadEvent> threadEvent;
	typedef std::list<std::wstring> lines_t;
	std::unique_ptr<lines_t> lines;
	bool bDoTimestamp;

	void Initialize(DWORD dwDelay);
	void LogLinesAndCleanup(std::unique_ptr<lines_t> data);
	void AllocateLines();

protected:
	virtual bool Write(const std::wstring& str);

public:
	CThreadedLogging(const std::wstring& dir, const std::wstring& file,
		const std::wstring& movedir,
		PhasorThread& thread, 
		DWORD dwDelay=DEFAULT_SAVE_DELAY);
	CThreadedLogging(const CLoggingStream& stream, PhasorThread& thread,
		DWORD dwDelay=DEFAULT_SAVE_DELAY);
	virtual ~CThreadedLogging();

	// CLoggingStream isn't threadsafe, so we're responsible for thread safety.
	virtual void SetMoveSize(DWORD kbSize);
	virtual void SetMoveDirectory(const std::wstring& move_to);
	virtual void SetOutFile(const std::wstring& directory,const std::wstring& fileName);
	virtual void SetOutFile(const std::wstring& fileName); // use cur dir
	virtual void EnableTimestamp(bool state);

	friend class CLogThreadEvent;
};

// Helper class which is invoked by PhasorThread when data can be saved.
class CLogThreadEvent : public PhasorThreadEvent
{
private:
	friend class CThreadLogging;
	CThreadedLogging& owner;

	CLogThreadEvent(CThreadedLogging& owner, DWORD dwDelay);

public:
	virtual void OnEventAux(PhasorThread& thread);

	friend class CThreadedLogging;
};