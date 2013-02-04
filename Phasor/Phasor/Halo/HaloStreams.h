#pragma once

#include "../../Common/Streams.h"
#include <assert.h>

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

	// Writes to a specific player
	class CPlayerStream : public COutStream
	{
	private:
		int memory_id;
		std::string hash;
		
		// used when cloning. forces checks to be made before each Write.
		CPlayerStream(const s_player& player, bool);

	protected:	
		const s_player& player;
		virtual bool Write(const std::wstring& str) override;

	public:
		CPlayerStream(const s_player& player)
			: player(player), memory_id(-1) {}
		virtual ~CPlayerStream() {}

		virtual std::unique_ptr<COutStream> clone() override
		{
			return std::unique_ptr<COutStream>(new CPlayerStream(player,true));
		}

		const s_player& GetPlayer() { return player; }
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
		std::unique_ptr<COutStream> clone() override
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

		std::unique_ptr<COutStream> clone() override
		{
			std::unique_ptr<COutStream> forwarder(new Forwarder);
			Forwarder* this_next = this, *that_next = (Forwarder*)forwarder.get();
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
}

