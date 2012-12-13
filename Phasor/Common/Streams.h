#pragma once

#include <string>
#include <sstream>
#include "Types.h"
#include "FileIO.h"

#define _WINDOWS_LINE_END
#ifndef _WINDOWS_LINE_END
static const wchar_t NEW_LINE_CHAR = L'\n';
#else
static const wchar_t* NEW_LINE_CHAR = L"\r\n";
#endif

struct endl_tag {};
static const endl_tag endl;

class CStreamModifier;

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
	//std::wstringstream ss;
	std::wstring str;
	static const size_t kDefaultBufferSize = 1 << 13; // 8kb

	void Reserve(size_t size);
protected:
	virtual bool Write(const std::wstring& str) = 0;

public: // stream modifiers
	bool no_flush;

public:
	COutStream();
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

	// Print using c-style functions. endl is appended to each message and
	// as such the stream is flushed after each call.
	void print(const char* format, ...);
	void wprint(const wchar_t* format, ...);
};

class NoFlush
{
private:
	COutStream& stream;
	bool prev;
public:
	explicit NoFlush(COutStream& stream) : stream(stream), 
		prev(stream.no_flush)
	{
		stream.no_flush = true;
	}

	~NoFlush()
	{
		stream.Flush();
		stream.no_flush = prev;
	}
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