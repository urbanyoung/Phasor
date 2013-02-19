#include "Objects.h"
#include "../Addresses.h"
#include "../../Globals.h"
#include "../tags.h"
#include "../Server/Server.h"
#include <map>

namespace halo { namespace objects {

	struct s_phasor_managed_obj
	{
		ident objid;
		bool bRecycle;
		vect3d pos, velocity, rotation, other;
		DWORD respawnTime, creationTicks;

		s_phasor_managed_obj(ident objid, bool bRecycle, const vect3d& pos, 
			DWORD respawnTime, s_object_creation_disposition* creation_disposition)
			: objid(objid), bRecycle(bRecycle), pos(pos), respawnTime(respawnTime)
		{
			s_halo_object* obj = (s_halo_object*)GetObjectAddress(objid);
			velocity = obj->velocity;
			rotation = obj->rotation;
			other = obj->someVector;
			creationTicks = server::GetServerTicks();
		}
	};
	// entry in the object table
	struct s_halo_object_header
	{
		WORD id;
		char flags; // 0x44 by default, dunno what they're for.
		char type;
		UNKNOWN(2);
		WORD size;
		union
		{
			void*					data;
			s_halo_object*	base;
			s_halo_biped*			biped;
		};
	};
	static_assert(sizeof(s_halo_object_header) == 0x0c, "s_halo_object_entry incorrect");

	struct s_halo_object_table
	{
		s_table_header header;
		s_halo_object_header entries[0x800];
	};	

	std::map<ident, std::unique_ptr<s_phasor_managed_obj>> managedList;

	// -------------------------------------------------------------------
	//
	void* GetObjectAddress(ident objectId)
	{
		s_halo_object_table* object_table = *(s_halo_object_table**)ADDR_OBJECTBASE;
		if (objectId.slot >= object_table->header.max_size) return 0;

		s_halo_object_header* obj = &object_table->entries[objectId.slot];
		return obj->id == objectId.id ? obj->data : 0;
	}

	// Called when an object is being checked to see if it should respawn
	int __stdcall ObjectRespawnCheck(ident m_objId, s_halo_object* obj)
	{
		return false;
	}

	// This is called when weapons/equipment are going to be destroyed.
	// todo: check ticks should be signed
	bool __stdcall EquipmentDestroyCheck(int checkTicks, ident m_objId, s_halo_object* obj)
	{
		return false;
	}

	bool SpawnObject(ident mapid, ident parentId, int respawnTime, bool bRecycle,
		const vect3d* location, ident& out_objid)
	{
		s_tag_entry* tag = LookupTag(mapid);
		if (!tag) return false;

		if (!parentId) parentId = make_ident(-1);

		// Build the request data for <halo>.CreateObject
		DWORD mapId = tag->id;
		BYTE query[0x100] = {0}; // i think max ever used is 0x90
		__asm
		{
			pushad
			push parentId
			push mapId
			lea eax, dword ptr ds:[query]
			call DWORD PTR ds:[FUNC_CREATEOBJECTQUERY]
			add esp, 8
			popad
		}

		s_object_creation_disposition* creation_disposition = 
			(s_object_creation_disposition*)query;

		// Set the spawn coordinates (if supplied)
		if (location)
			creation_disposition->pos = *location;
		else 
			location = &creation_disposition->pos;

		// Create the object
		__asm
		{
			pushad
			push 0
			lea eax, dword ptr ds:[query]
			push eax
			call DWORD PTR ds:[FUNC_CREATEOBJECT]
			add esp, 8
			mov out_objid, eax
			popad
		}

		if (!out_objid.valid()) return false;

		// resolve the respawn timer
		if (respawnTime == -1) // use gametype's value
		{
			// weapons/equipment don't respawn, they're destroyed and
			// so the default value is handled in the processing func
			if (tag->tagType == "eqip" || tag->tagType == "weap")
				respawnTime = -1;
			else
				respawnTime = server::GetRespawnTicks();
		}
		else
			respawnTime *= 30;

		std::unique_ptr<s_phasor_managed_obj> obj(
			new s_phasor_managed_obj(out_objid, bRecycle, *location, respawnTime,
			creation_disposition)
			);

		managedList.insert(std::pair<ident, std::unique_ptr<s_phasor_managed_obj>>(out_objid, std::move(obj)));
	
		return true;
	}

	// --------------------------------------------------------------------
	//
	void s_halo_weapon::SetAmmo(WORD pack, WORD clip)
	{
		ammo_pack = pack;
		ammo_clip = clip;
	}

	void s_halo_weapon::SyncAmmo(ident weaponId)
	{
		DWORD id = weaponId;
		__asm
		{
			pushad
				mov ebx, 0
				mov ecx, id
				call dword ptr ds:[FUNC_UPDATEAMMO]
			popad
		}
	}
}}