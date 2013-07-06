#include "../../../Common/Streams.h"
#include "../../Commands.h"

namespace halo { namespace server { namespace misc { 
	// ------------------------------------------------------------
	// Version commands
	e_command_result sv_version(void*, 
		commands::CArgParser& args, COutStream& out);

	e_command_result sv_version_check(void*, 
		commands::CArgParser& args, COutStream& out);
}}}