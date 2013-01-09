#pragma once

#include "../../../Common/Streams.h"

namespace halo { namespace server
{
	class CHaloPrintStream : public COutStream
	{
	protected:
		virtual bool Write(const std::wstring& str);

	public:
		CHaloPrintStream() {}
		virtual ~CHaloPrintStream() {}

		virtual std::unique_ptr<COutStream> clone()
		{
			return std::unique_ptr<COutStream>(new CHaloPrintStream());
		}
	};

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

		virtual std::unique_ptr<COutStream> clone()
		{
			return std::unique_ptr<COutStream>(new CEchoStream(
				first.clone(), second.clone()));
		}
	};
}}

