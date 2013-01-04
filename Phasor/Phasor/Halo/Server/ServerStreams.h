#pragma once

#include "../../../Common/Streams.h"

namespace halo { namespace server
{
	class CHaloEchoStream;
	class CHaloPrintStream : public COutStream
	{
	protected:
		virtual bool Write(const std::wstring& str);

	public:
		CHaloPrintStream() {}
		virtual ~CHaloPrintStream() {}
	};

	class CEchoStream : public COutStream
	{
	private:
		COutStream &first, &second;
	protected:		   
		virtual bool Write(const std::wstring& str);

	public:
		CEchoStream(COutStream& first, COutStream& second)
			: first(first), second(second) {}
		virtual ~CEchoStream() {}
	};
}}

