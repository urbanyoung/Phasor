#include "haloobjects.h"
#include "api_readers.h"
#include "../Phasor/Halo/Game/Objects.h"
#include "../Phasor/Halo/Game/Game.h"

using namespace Common;
using namespace Manager;
using namespace halo::objects;
using namespace halo;

void l_getobject(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	ident id = make_ident(ReadNumber<DWORD>(*args[0]));
	void* object = GetObjectAddress(id);
	if (object) AddResultPtr(object, results);
	else AddResultNil(results);
}

void l_getobjectcoords(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	ident id = make_ident(ReadNumber<DWORD>(*args[0]));
	s_halo_object* obj = (s_halo_object*)GetObjectAddress(id);
	if (!obj) handler.RaiseError("getobjectcoords : invalid object id");
	AddResultNumber(obj->location.x, results);
	AddResultNumber(obj->location.y, results);
	AddResultNumber(obj->location.z, results);
}

void l_objecttoplayer(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	void* obj_addr = (void*)ReadNumber<DWORD>(*args[0]);
	for (int i = 0; i < 16; i++) {
		s_player* player = game::GetPlayer(0);
		if (player && GetObjectAddress(player->mem->object_id) == obj_addr) {
			AddResultNumber(i, results);
			return;
		}
	}
	AddResultNil(results);
}