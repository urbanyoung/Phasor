#include "../../../Common/Streams.h"
#include "../../Commands.h"

namespace halo { namespace server { namespace misc { 
	// ------------------------------------------------------------
	// Version commands
	e_command_result sv_version(void*, 
		commands::CArgParser& args, COutStream& out);

	e_command_result sv_version_check(void*, 
		commands::CArgParser& args, COutStream& out);

	// Hash checking
	e_command_result sv_hash_check(void*, 
		commands::CArgParser& args, COutStream& out);

	// Other
	e_command_result sv_kill(void*, 
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_getobject(void*, 
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_invis(void*, 
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_setspeed(void*, 
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_say(void*, 
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_gethash(void*, 
		commands::CArgParser& args, COutStream& out);
	e_command_result sv_changeteam(void*, 
		commands::CArgParser& args, COutStream& out);


}}}