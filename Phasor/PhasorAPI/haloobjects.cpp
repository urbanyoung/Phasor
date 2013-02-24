#include "haloobjects.h"
#include "api_readers.h"
#include "../Phasor/Halo/Game/Objects.h"
#include "../Phasor/Halo/Game/Game.h"
#include "../Phasor/Halo/tags.h"

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

void l_createobject(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	s_tag_entry* tag;

	size_t i = 0;
	if (args.size() == 8) {
		std::string tagtype = ReadRawString(*args[i++]);
		std::string tagname = ReadRawString(*args[i++]);
		tag = LookupTag(s_tag_type(tagtype.c_str()), tagname);
	} else {
		// never used atm.. need to add type overloading first.
		/*! \todo add type overloading to phasorapi args */
		DWORD tag_id = ReadNumber<DWORD>(*args[i++]);
		tag = LookupTag(make_ident(tag_id));
	}

	DWORD parentId = ReadNumber<DWORD>(*args[i++]);
	if (parentId == 0) parentId = 0xFFFFFFFF;
	int respawnTime = ReadNumber<int>(*args[i++]);
	bool do_respawn = ReadBoolean(*args[i++]);
	vect3d pos;
	pos.x = ReadNumber<float>(*args[i++]);
	pos.y = ReadNumber<float>(*args[i++]);
	pos.z = ReadNumber<float>(*args[i++]);
	
	if (!tag) handler.RaiseError("createobject : cannot find object to create.");

	ident objid;
	if (CreateObject(tag->id, make_ident(parentId), respawnTime, do_respawn, &pos, objid))
		AddResultNumber((unsigned long)objid, results);
	else 
		AddResultNil(results);
}

void l_destroyobject(CallHandler& handler, Object::unique_deque& args, Object::unique_list&)
{
	ident objid = make_ident(ReadNumber<DWORD>(*args[0]));
	if (!DestroyObject(objid)) handler.RaiseError("destroyobject : invalid object id.");
}

void l_assignweapon(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	s_player* player = ReadPlayer(handler, *args[0], true);
	ident weapId = make_ident(ReadNumber<DWORD>(*args[1]));

	AddResultBool(AssignPlayerWeapon(*player, weapId), results);
}

void l_entervehicle(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	s_player* player = ReadPlayer(handler, *args[0], true);
	ident vehicleId = make_ident(ReadNumber<DWORD>(*args[1]));
	DWORD seat = ReadNumber<DWORD>(*args[2]);

	if (!EnterVehicle(*player, vehicleId, seat)) 
		handler.RaiseError("entervehicle : invalid vehicle id.");
}

void l_isinvehicle(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	s_player* player = ReadPlayer(handler, *args[0], true);
	AddResultBool(player->InVehicle(), results);
}

void l_exitvehicle(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	s_player* player = ReadPlayer(handler, *args[0], true);
	if (!ExitVehicle(*player)) handler.RaiseError("exitvehicle : player not in vehicle.");
}

void l_movobjectcoords(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	ident objid = make_ident(ReadNumber<DWORD>(*args[0]));
	vect3d pos;
	pos.x = ReadNumber<float>(*args[1]);
	pos.y = ReadNumber<float>(*args[2]);
	pos.z = ReadNumber<float>(*args[3]);

	s_halo_object* obj = (s_halo_object*)GetObjectAddress(objid);
	if (!obj) handler.RaiseError("movobjectcoords : invalid object id.");
	MoveObject(*obj, pos);
}

void l_gettagid(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	std::string tagtype = ReadRawString(*args[0]);
	std::string tagname = ReadRawString(*args[1]);
	s_tag_entry* tag = LookupTag(s_tag_type(tagtype.c_str()), tagname);
	if (!tag) AddResultNil(results);
	else
		AddResultNumber(tag->id, results);
}

void l_gettagaddress(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	ident tagid = make_ident(ReadNumber<DWORD>(*args[0]));
	s_tag_entry* tag = LookupTag(tagid);
	if (!tag) handler.RaiseError("gettagaddress : invalid tag id");
	AddResultPtr(tag, results);
}