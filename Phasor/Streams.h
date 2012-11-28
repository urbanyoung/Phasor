#pragma once

#include <string>
#include <sstream>
#include "Types.h"
#include "FileIO.h"

// comment this out if you want to change the line ending
#define _WINDOWS_LINE_END
#ifndef _WINDOWS_LINE_END
static const wchar_t NEW_LINE_CHAR = L'\n';
#else
static const wchar_t* NEW_LINE_CHAR = L"\r\n";
#endif

struct endl_tag {};
static const endl_tag endl;

class CInStream
{
public:
	CInStream();
	virtual ~CInStream();

	// Reads data from the stream.
	// 
	virtual bool Read(BYTE* outbuffer, DWORD to_read, DWORD* read) = 0;
};

class COutStream
{
private:
	std::wstringstream ss;

	virtual bool Write(const std::wstring& str) = 0;
public:
	virtual ~COutStream();

	// Called to flush the stream (and on endl)
	void Flush();

	COutStream & operator<<(const endl_tag&);
	COutStream & operator<<(const std::string& string);
	COutStream & operator<<(const std::wstring& string);
	COutStream & operator<<(const char *string);
	COutStream & operator<<(const wchar_t *string);
	COutStream & operator<<(wchar_t c);
	COutStream & operator<<(DWORD number);
	COutStream & operator<<(int number);
	COutStream & operator<<(double number);
};

// Output to a file which is kept open for the duration of the stream
class COutFileStream : public COutStream
{
protected:
	COutFile file;

public:
	COutFileStream();
	virtual ~COutFileStream();

	// Open the specified file, creating it if it doesn't exist.
	bool Open(const std::wstring& file);

	virtual bool Write(const std::wstring& str);
};

// Output to a log file which is only opened when writing to it
class CLoggingStream : public COutStream
{
private:

	std::wstring moveDirectory; // directory to move file to
	std::wstring filePath; // path to file inclusive of name and extension
	std::wstring fileName; //  name of file (no path info, no extension)
	std::wstring fileExtension;
	DWORD byteSize; // max size for file

	// Check if the file should be moved.
	void CheckAndMove(DWORD curSize, const SYSTEMTIME& st);

	virtual bool Write(const std::wstring& str);

public:
	CLoggingStream(const std::wstring& file);
	virtual ~CLoggingStream();

	void SetMoveInfo(const std::wstring& move_to, DWORD kbSize);	
};