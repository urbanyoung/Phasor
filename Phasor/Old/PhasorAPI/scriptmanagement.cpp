#include "scriptmanagement.h"
#include "api_readers.h"

using namespace Common;
using namespace Manager;

void l_raiseerror(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	handler.RaiseError(ReadRawString(*args[0]));
}