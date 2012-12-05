#pragma once

#include "../Common/Streams.h"
#include <memory>

// Stream which output data to a file with .log extension and timestamps
// each entry.
class CLoggingStream : public COutStream
{
private:
	std::wstring moveDirectory; // directory to move file to
	std::wstring filePath; // path to file inclusive of name and extension
	std::wstring fileName; //  name of file (no path info, no extension)
	std::wstring fileDirectory;

	DWORD byteSize; // max size for file before being moved
	DWORD errorOffset; // increases when moving the file fails

	void Initialize(const std::wstring& directory,
		const std::wstring& fileName);
	void SetNames(const std::wstring& directory,
		const std::wstring& fileName);

	// Check if the file should be moved.
	void CheckAndMove(DWORD curSize, const SYSTEMTIME& st);

protected:
	virtual bool Write(const std::wstring& str);

public:
	// directory should be normalized (finish with a single \\)
	CLoggingStream(const std::wstring& dir, const std::wstring& file);
	CLoggingStream(const CLoggingStream& other);
	virtual ~CLoggingStream();

	void SetMoveInfo(const std::wstring& move_to, DWORD kbSize);
	void SetOutFile(const std::wstring& directory,const std::wstring& fileName);
	void SetOutFile(const std::wstring& fileName); // use cur dir
};

typedef CLoggingStream CPhasorLog;
typedef CLoggingStream CScriptsLog;

extern std::unique_ptr<CScriptsLog> g_ScriptsLog;
extern std::unique_ptr<CPhasorLog> g_PhasorLog;
