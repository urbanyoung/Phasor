#include "Objects.h"
#include "../Addresses.h"
#include "../../Globals.h"
#include "../Server/Server.h"
#include <array>
#include <boost/optional.hpp>

namespace halo { namespace objects {

	struct s_phasor_managed_obj
	{
		ident objid;
		bool bRecycle;
		vect3d pos, velocity, rotation, other;
		boost::optional<int> respawnTicks;
		DWORD creationTicks;

		s_phasor_managed_obj(ident objid, bool bRecycle, const vect3d& pos, 
			boost::optional<int> respawnTicks, s_object_creation_disposition* creation_disposition)
			: objid(objid), bRecycle(bRecycle), pos(pos), respawnTicks(respawnTicks)
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
		char flag1; // 0x44 by default, dunno what they're for.
		char flag2;
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

    struct ManagedObjects {
        std::array<boost::optional<s_phasor_managed_obj>, 0x800> objs;

        void clear() {
            objs.fill(boost::none);
        }

        inline void remove(ident id) {
            objs[id.slot] = boost::none;
        }

        inline boost::optional<s_phasor_managed_obj>& find(ident id) {
            return objs[id.slot];
        }

        inline void add(s_phasor_managed_obj obj) {
            objs[obj.objid.slot] = std::move(obj);
        }

    } managed;
    

	void ClearManagedObjects() {
        managed.clear();
	}

	// -------------------------------------------------------------------
	//
	void* GetObjectAddress(ident objectId)
	{
		s_halo_object_table* object_table = *(s_halo_object_table**)ADDR_OBJECTBASE;
		if (objectId.slot >= object_table->header.max_size) return 0;

		s_halo_object_header* obj = &object_table->entries[objectId.slot];
        //if (obj->flag1 & 8) return 0; // due to be destroyed
		return obj->id == objectId.id ? obj->data : 0;
	}

	bool DestroyObject(ident objid)
	{
		if (!GetObjectAddress(objid)) return false;
		__asm {
			pushad
			mov eax, objid
			call dword ptr ds:[FUNC_DESTROYOBJECT]
			popad
		}
		return true;
	}


	// Called when an object is being destroyed
	void OnObjectDestroy(ident m_objid) {
        managed.remove(m_objid);
	}

	// Called when an object is being checked to see if it should respawn
	// Return values:	0 - don't respawn
	//					1 - respawn
	//					2 - object destroyed
	int __stdcall VehicleRespawnCheck(ident m_objId, s_halo_vehicle* obj)
	{
		if (obj->idle_timer == 0xffffffff) return 0; // still active

		DWORD respawn_ticks = server::GetRespawnTicks();
		DWORD server_ticks = server::GetServerTicks();

		int result = 0;

        boost::optional<s_phasor_managed_obj>& mobj = managed.find(m_objId);
		if (mobj) {
			s_phasor_managed_obj& phasorObj = *mobj;
            if (phasorObj.respawnTicks) { // unset when no respawn
                DWORD expiration = obj->idle_timer + *phasorObj.respawnTicks;
				if (expiration < server_ticks) {
                    if (phasorObj.bRecycle) {
                        void* v1 = &phasorObj.other, *rotation = &phasorObj.rotation,
                            *position = &phasorObj.pos;

						__asm {
							pushad
							push m_objId
							call dword ptr ds : [FUNC_VEHICLERESPAWN2] // set flags to let object fall, reset velocities etc
							add esp, 4
							push v1
							push rotation
							push m_objId
							mov edi, position
							call dword ptr ds : [FUNC_VEHICLERESPAWN1] // move the object (proper way)
							add esp, 0x0c
							popad
						}

						// set last interacted to be now
						obj->idle_timer = server_ticks;
					} else { // destroy
						DestroyObject(m_objId);
						result = 2;
					}
				}
			}
		} else if (respawn_ticks != 0) { // default processing
			DWORD expiration = obj->idle_timer + respawn_ticks;
			if (expiration < server_ticks) result = 1;
		}
		return result;
	}
	
	// This is called when weapons/equipment are going to be destroyed.
	// todo: check ticks should be signed
	bool __stdcall EquipmentDestroyCheck(int checkTicks, ident m_objId, s_halo_object* obj)
	{
		bool bDestroy = false;
		int objTicks = *(int*)((LPBYTE)obj + 0x204);

        boost::optional<s_phasor_managed_obj>& mobj = managed.find(m_objId);
        if (mobj) {
			s_phasor_managed_obj& phasorObj = *mobj;

			// respawn ticks are treated as expiration ticks
            if (phasorObj.respawnTicks) {
                if (*phasorObj.respawnTicks > 0) {
                    if (*phasorObj.respawnTicks + (int)phasorObj.creationTicks < checkTicks)
						bDestroy = true;
				} else { // use default value
					if (checkTicks > objTicks)
						bDestroy = true;
				}
			}			
		} else {
			if (checkTicks > objTicks) 
				bDestroy = true;
		}

		return bDestroy;
	}

	bool CreateObject(s_tag_entry* tag, ident parentId,
		boost::optional<int> respawnTime,
		bool bRecycle,
		const vect3d* location, ident& out_objid)
	{
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

		DWORD objid;
		// Create the object
		__asm
		{
			pushad
			push 0
			lea eax, dword ptr ds:[query]
			push eax
			call DWORD PTR ds:[FUNC_CREATEOBJECT]
			add esp, 8
			mov objid, eax
			popad
		}
		out_objid = make_ident(objid);
		if (!out_objid.valid())	return false;

		// resolve the respawn timer
		if (respawnTime == -1) { // use gametype's value
			// weapons/equipment don't respawn, they're destroyed and
			// so the default value is handled in the processing func
			if (tag->tagType == TAG_EQIP || tag->tagType == TAG_WEAP)
				respawnTime = -1;
			else
				respawnTime = server::GetRespawnTicks();
		} else if (respawnTime) {
			*respawnTime *= 30;
		}

        managed.add(s_phasor_managed_obj(out_objid, bRecycle, *location, respawnTime,
            creation_disposition));
	
		return true;
	}

	// Forces a player to equip (and change to) the specified weapon.
	bool AssignPlayerWeapon(s_player& player, ident weaponid)
	{
		bool bSuccess = false;

		s_halo_biped* biped = player.get_object();
		if (!biped) return false;

		// can't be in vehicle
		if (!biped->base.vehicleId.valid())	{
			// make sure they passed a weapon
			s_halo_weapon* weapon = (s_halo_weapon*)GetObjectAddress(weaponid);
			if (!weapon || weapon->base.objectType != e_object_type::weapon) return false;
			
			s_tag_entry* weap_tag = LookupTag(weapon->base.map_id);
			if (weap_tag->tagType != TAG_WEAP) return false;

			DWORD mask = player.getPlayerIdent();
			ident playerObj = player.mem->object_id;

			__asm
			{
				pushad
				push 1
				mov eax, weaponid
				mov ecx, playerObj
				call dword ptr ds:[FUNC_PLAYERASSIGNWEAPON] // assign the weapon
				add esp, 4
				mov bSuccess, al
				cmp al, 1
				jnz ASSIGNMENT_FAILED
				mov ecx, mask
				mov edi, weaponid
				push -1
				push -1
				push 7
				push 1
				call DWORD PTR ds:[FUNC_NOTIFY_WEAPONPICKUP] // notify clients of the assignment
				add esp, 0x10
ASSIGNMENT_FAILED:
				popad
			}
		}

		return bSuccess;
	}

	// Forces a player into a vehicle
	// Seat numbers: 0 (driver) 1 (passenger) 2 (gunner)
	bool EnterVehicle(s_player& player, ident m_vehicleId, DWORD seat)
	{
		s_halo_vehicle* vehicle = (s_halo_vehicle*)GetObjectAddress(m_vehicleId);
		if (!vehicle || vehicle->base.objectType != e_object_type::vehicle) return false; 

		// set interaction info
		player.mem->m_interactionObject = m_vehicleId;
		player.mem->interactionType = 8; // vehicle interaction
		player.mem->interactionSpecifier = (WORD)seat;
		player.force_entered = true; // so scripts cant stop them entering
		DWORD mask = player.getPlayerIdent();

		// enter the vehicle (if seat is free)
		__asm
		{
			pushad
			push mask
			call dword ptr ds:[FUNC_ENTERVEHICLE]
			add esp, 4
			popad
		}

		player.force_entered = false;
		return true;
	}

	// Forces a player to exit a vehicle
	bool ExitVehicle(s_player& player)
	{
		if (!player.InVehicle()) return false;

		DWORD playerObj = player.mem->object_id;
		__asm
		{
			pushad
			mov eax, playerObj
			call dword ptr ds:[FUNC_EJECTVEHICLE]
			popad
		}
	}

	void MoveObject(s_halo_object& object, const vect3d& pos)
	{
		object.location = pos;
		object.stationary = false;
	}

	bool FindIntersection(const view_vector& view, const halo::ident& ignore_obj,
		vect3d& hit_pos, ident& hit_obj)
	{	
		const vect3d* dir = &view.dir, *pos = &view.pos;
		s_intersection_output result;
		bool hit;
		DWORD ignore = ignore_obj;

		__asm {
			pushad

			lea eax, result
			push eax
			push ignore
			push dir
			push pos
			push 0x1000E9
			call dword ptr ds:[FUNC_INTERSECT]
			mov hit, al
			add esp, 0x14

			popad
		}

		hit_pos = result.hit_pos;
		if (result.mode == 3) hit_obj = result.hit_obj;

		return hit;
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