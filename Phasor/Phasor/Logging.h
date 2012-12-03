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

	DWORD byteSize; // max size for file
	DWORD errorOffset; // increases when moving the file fails

	void Initialize(const std::wstring& file, bool extension=true);

	// Check if the file should be moved.
	void CheckAndMove(DWORD curSize, const SYSTEMTIME& st);

protected:
	virtual bool Write(const std::wstring& str);

public:
	CLoggingStream(const std::wstring& dir, const std::wstring& file);
	CLoggingStream(const std::wstring& file);
	CLoggingStream(const CLoggingStream& other);
	virtual ~CLoggingStream();

	void SetMoveInfo(const std::wstring& move_to, DWORD kbSize);	
};
