#include "haloobjects.h"
#include "api_readers.h"
#include "../Phasor/Halo/Game/Objects.h"
using namespace Common;
using namespace Manager;

void l_getobject(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	using namespace halo::objects;
	using namespace halo;

	ident id = make_ident(ReadNumber<DWORD>(*args[0]));
	void* object = GetObjectAddress(id);
	if (object) AddResultPtr(object, results);
	else AddResultNil(results);
}