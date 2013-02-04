#pragma once

#include <string>
#include <sstream>
#include <memory>
#include "noncopyable.h"
#include "Types.h"

#define _WINDOWS_LINE_END
#ifndef _WINDOWS_LINE_END
static const wchar_t NEW_LINE_CHAR = L'\n';
#else
static const wchar_t* NEW_LINE_CHAR = L"\r\n";
#endif

struct endl_tag {};
static const endl_tag endl;

class CStreamModifier;

class COutStream : private noncopyable
{
private:
	std::wstring str;
	size_t length_to_write;
	static const size_t kDefaultBufferSize = 1 << 13; // 8kb

protected:
	
	// These are the only functions to access str and length_to_write.
	// Derived classes that need to provide thread safety should override these.
	virtual void AppendData(const std::wstring& str);
	virtual void AppendData(wchar_t c);
	virtual void Reserve(size_t size);

public: // stream modifiers
	bool no_flush;

public:
	COutStream();
	virtual ~COutStream();

	// Creates a copy of stream
	virtual std::unique_ptr<COutStream> clone() = 0;
	// Called to write data to the stream
	virtual bool Write(const std::wstring& str) = 0;

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
		if ((stream.no_flush = prev)) stream.Flush();		
	}
};