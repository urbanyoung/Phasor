#pragma once

#include <string>
#include <sstream>
#include <memory>
#include <assert.h>
#include <list>
#include <set>
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
	std::set<COutStream*> masters;

	// masters should be valid for the duration of this stream
	void Notify(COutStream& master);
	void DontNotify(COutStream& master);

public:
	COutStream();
	virtual ~COutStream();

	// Creates a copy of stream
	virtual std::unique_ptr<COutStream> clone() const = 0;
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
		if (!(stream.no_flush = prev)) stream.Flush();		
	}
};

class NotifyStream
{
private:
	COutStream& slave, &master;

public:
	NotifyStream(COutStream& slave, COutStream& master)
		: slave(slave), master(master)
	{
		slave.Notify(master);
	}

	~NotifyStream()
	{
		slave.DontNotify(master);
	}
};

// Creates a temporary forwarding chain
// No streams are copied and as such this class cannot be copied/cloned
// and all streams should remain valid for its duration.
class TempForwarder : public COutStream
{
public:
	typedef std::unique_ptr<TempForwarder> next_ptr;
private:
	COutStream& stream;
	next_ptr next;

protected:
	bool Write(const std::wstring& str) override
	{
		bool b = true;
		if (next) b = next->Write(str);		
		return b && stream.Write(str);
	}
	// This stream is temporary and shouldn't be copied/cloned.
	std::unique_ptr<COutStream> clone() const override
	{
		assert(0);
		return std::unique_ptr<COutStream>();
	}

public:
	TempForwarder(COutStream& stream, next_ptr& next)
		: stream(stream), next(std::move(next))	{}		

	static next_ptr end_point(COutStream& stream)
	{
		return next_ptr(new TempForwarder(stream, next_ptr()));
	}

	static next_ptr mid_point(COutStream& stream, next_ptr& next)
	{
		return next_ptr(new TempForwarder(stream, std::move(next)));
	}
};

// Used to create a forwarding chain for COutStreams.
// All streams passed in are cloned and managed by this class.
class Forwarder : public COutStream
{
protected:
	bool Write(const std::wstring& str) override
	{
		bool b = true;
		if (next) b = next->Write(str);		
		return b && stream->Write(str);
	}

public:
	typedef std::unique_ptr<Forwarder> next_ptr;
	typedef std::unique_ptr<COutStream> stream_ptr;

	explicit Forwarder(COutStream& stream, next_ptr& next)
		: stream(stream.clone()), next(std::move(next))
	{
	}

	std::unique_ptr<COutStream> clone() const override
	{
		std::unique_ptr<COutStream> forwarder(new Forwarder);
		const Forwarder* this_next = this;
		Forwarder* that_next = (Forwarder*)forwarder.get();
		while (this_next) {
			that_next->next = next_ptr((Forwarder*)this_next->next->clone().release());
			that_next->stream = this_next->stream->clone();
			this_next = this->next->next.get();
			that_next = that_next->next.get();
		}
		return forwarder;
	}

	static next_ptr end_point(COutStream& stream)
	{
		return next_ptr(new Forwarder(stream, next_ptr()));
	}

	static next_ptr mid_point(COutStream& stream, next_ptr& next)
	{
		return next_ptr(new Forwarder(stream, std::move(next)));
	}

private:
	next_ptr next;
	stream_ptr stream;

	Forwarder() {}
};


// ignores all input
class SinkStream : public COutStream
{
	virtual bool Write(const std::wstring&)
	{
		return true;
	}

public:

	virtual std::unique_ptr<COutStream> clone() const override
	{
		return std::unique_ptr<COutStream>(new SinkStream());
	}

};

class RecordStream : public COutStream
{
private:
	std::list<std::wstring> output;

protected:
	virtual bool Write(const std::wstring& str)
	{
		if (str.size() != 0) output.push_back(str);
		return true;
	}

public:
	RecordStream() {}

	const std::list<std::wstring>& getRecord() const {
		return output;
	}

	virtual std::unique_ptr<COutStream> clone() const override
	{
		return std::unique_ptr<COutStream>(new RecordStream());
	}
};