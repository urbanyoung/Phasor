#pragma once

#include "../../Common/Streams.h"

namespace halo { 
	struct s_player;

	// Writes to the server console.
	class CHaloPrintStream : public COutStream
	{
	protected:
		virtual bool Write(const std::wstring& str);

	public:
		CHaloPrintStream() {}
		virtual ~CHaloPrintStream() {}

		virtual std::unique_ptr<COutStream> clone() override
		{
			return std::unique_ptr<COutStream>(new CHaloPrintStream());
		}
	};

	// Writes to two streams at once. 
	class CEchoStream : public COutStream
	{
	private:
		COutStream &first, &second;
		std::unique_ptr<COutStream> first_ptr, second_ptr;

		// If an echo stream gets cloned it needs to own the clones of
		// first and second.
		CEchoStream(std::unique_ptr<COutStream> first_ptr, 
			std::unique_ptr<COutStream> second_ptr)
			: first(*first_ptr), second(*second_ptr),
			first_ptr(std::move(first_ptr)), second_ptr(std::move(second_ptr))
		{
		}

	protected:		   
		virtual bool Write(const std::wstring& str);

	public:
		CEchoStream(COutStream& first, COutStream& second)
			: first(first), second(second) {}
		virtual ~CEchoStream() {}

		virtual std::unique_ptr<COutStream> clone() override
		{
			return std::unique_ptr<COutStream>(new CEchoStream(
				first.clone(), second.clone()));
		}
	};

	// Writes to a specific player
	class CPlayerStream : public COutStream
	{
	protected:	
		const s_player& player;
		virtual bool Write(const std::wstring& str) override;

	public:
		CPlayerStream(const s_player& player)
			: player(player) {}
		virtual ~CPlayerStream() {}

		virtual std::unique_ptr<COutStream> clone() override
		{
			return std::unique_ptr<COutStream>(new CPlayerStream(player));
		}

		const s_player& GetPlayer() { return player; }
	};	

	// Wrapper class around COutStreams. When being cloned non-player
	// streams are treated as normal (virtual clone method called) whereas
	// player streams get transformed into a CCheckedPlayerStream which
	// checks the player still exists before writing to it.
	class CCheckedStream : noncopyable
	{
	private:
		COutStream& stream;
		bool player_stream;
	public:
		CCheckedStream(COutStream& stream, bool player_stream) 
			: stream(stream), player_stream(player_stream)
		{
		}

		COutStream& operator()()
		{
			return stream;
		}

		operator COutStream&(void) const
		{
			return stream;
		}

		// Proxy COutStream
		COutStream & operator<<(const endl_tag&) { return stream << endl; }
		COutStream & operator<<(const std::string& string) { return stream << string; }
		COutStream & operator<<(const std::wstring& string) { return stream << string; }
		COutStream & operator<<(const char *string) { return stream << string; }
		COutStream & operator<<(const wchar_t *string) { return stream << string; }
		COutStream & operator<<(wchar_t c) { return stream << c; }
		COutStream & operator<<(DWORD number) { return stream << number; }
		COutStream & operator<<(int number)  { return stream << number; }
		COutStream & operator<<(double number){ return stream << number; }

		std::unique_ptr<COutStream> clone_stream();
	};
}

