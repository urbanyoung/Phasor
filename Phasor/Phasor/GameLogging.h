#pragma once

#include "ThreadedLogging.h"

enum glog_type
{
	kGameEnd = 1 << 6,
	kGameStart,
	kPlayerJoin = (1 << 7) + 2,
	kPlayerLeave,
	kPlayerChange,
	kPlayerDeath,
	kServerCommand = (1 << 8) + 6,
	kServerClose
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