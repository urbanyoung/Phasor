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
	// cs locks lines, loggingStreamCS locks the stream
	CRITICAL_SECTION cs, loggingStreamCS;
	DWORD id; // timer id
	PhasorThread& thread;
	std::shared_ptr<CLogThreadEvent> threadEvent;
	typedef std::list<std::wstring> lines_t;
	std::unique_ptr<lines_t> lines;
	bool bDoTimestamp;
	DWORD dwDelay;

	void Initialize(DWORD dwDelay);
	// Write the lines in data to file, then cleanup data.
	void LogLinesAndCleanup(std::unique_ptr<lines_t> data);
	// Reallocate lines.
	void AllocateLines();

protected:
	// CLoggingStream
	bool Write(const std::wstring& str) override;

public:
	CThreadedLogging(const std::wstring& dir, const std::wstring& file,
		const std::wstring& movedir,
		PhasorThread& thread, 
		DWORD dwDelay=DEFAULT_SAVE_DELAY);
	CThreadedLogging(const CLoggingStream& stream, PhasorThread& thread,
		DWORD dwDelay=DEFAULT_SAVE_DELAY);
	~CThreadedLogging();

	// COutStream
	std::unique_ptr<COutStream> clone() const override;

	// CLoggingStream isn't threadsafe, so we're responsible for thread safety.
	void SetMoveSize(DWORD kbSize) override;
	void SetMoveDirectory(const std::wstring& move_to) override;
	void SetOutFile(const std::wstring& directory,const std::wstring& fileName) override;
	void SetOutFile(const std::wstring& fileName) override; // use cur dir
	void EnableTimestamp(bool state) override;

	// COutStream isn't thread safe either.
	void AppendData(const std::wstring& str) override;
	void AppendData(wchar_t c) override;
	void Reserve(size_t size) override;

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
	// PhasorThreadEvent
	void OnEventAux(PhasorThread& thread) override;

	friend class CThreadedLogging;
};