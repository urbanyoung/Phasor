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
		PhasorThread& thread, 
		DWORD dwDelay=DEFAULT_SAVE_DELAY);
	CThreadedLogging(const CLoggingStream& stream, PhasorThread& thread,
		DWORD dwDelay=DEFAULT_SAVE_DELAY);
	virtual ~CThreadedLogging();

	// CLoggingStream isn't threadsafe, so we're responsible for thread safety.
	virtual void SetMoveInfo(const std::wstring& move_to, DWORD kbSize);
	virtual void SetOutFile(const std::wstring& directory,const std::wstring& fileName);
	virtual void SetOutFile(const std::wstring& fileName); // use cur dir
	virtual void EnableTimestamp(bool state);
	virtual std::wstring PrependTimestamp(const std::wstring& str);

	friend class CLogThreadEvent;
};

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