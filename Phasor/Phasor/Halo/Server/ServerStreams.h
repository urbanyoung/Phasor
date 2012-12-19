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
		friend class CHaloEchoStream;
	};

	class CHaloEchoStream : public COutStream
	{
	private:
		CHaloPrintStream printStream;	
		COutStream& logStream;
	protected:		   
		virtual bool Write(const std::wstring& str);

	public:
		CHaloEchoStream(COutStream& logStream) : logStream(logStream) {}
		virtual ~CHaloEchoStream() {}
	};
}}

