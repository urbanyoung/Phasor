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
	};
}}

extern halo::server::CHaloPrintStream g_PrintStream;