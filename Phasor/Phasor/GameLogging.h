#pragma once

#include "ThreadedLogging.h"

// These are encoded as 8 bit numbers
// xxxx xxxx
// type event
// Where type is Game, Player etc and event is the specific event (join, leave etc)

enum glog_type
{
	kGameEnd = 1 << 4,
	kGameStart,
	kPlayerJoin = (1 << 5) + 2,
	kPlayerLeave,
	kPlayerChange,
	kPlayerDeath,
	kPlayerChat,
	kServerCommand = (1 << 6) + 6,
	kServerClose,
	kScriptEvent = (1 << 7) + 8
};

class CGameLog
{
private:
	std::unique_ptr<CThreadedLogging> logstream;
	static const unsigned int kSaveDelay = 10000; // 10 seconds

public:
	CGameLog(const std::wstring& dir, const std::wstring& file,
		PhasorThread& thread);

	// linebreak is automatically added
	void WriteLog(glog_type type, wchar_t* format, ...);
	void WriteLog(glog_type type, char* format, ...);

	CLoggingStream& GetLogStream() { return *logstream; }
};