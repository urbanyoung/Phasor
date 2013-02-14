#include "Objects.h"
#include "../Addresses.h"
#include "../../Globals.h"

namespace halo { namespace objects {

	// entry in the object table
	struct s_halo_object
	{
		WORD id;
		char flags; // 0x44 by default, dunno what they're for.
		char type;
		UNKNOWN(2);
		WORD size;
		union
		{
			void*					data;
			s_halo_object_header*	base;
			s_halo_biped*			biped;
		};
	};
	static_assert(sizeof(s_halo_object) == 0x0c, "s_halo_object_entry incorrect");

	/*! \todo look into expanding object limit.. probably not possible due to clients tho */
	struct s_halo_object_table
	{
		s_table_header header;
		s_halo_object entries[0x800];
	};	

	/*! \todo check this works */
	void* GetObjectAddress(ident objectId)
	{
		s_halo_object_table* object_table = *(s_halo_object_table**)ADDR_OBJECTBASE;
		if (objectId.slot >= object_table->header.max_size) return 0;

		s_halo_object* obj = &object_table->entries[objectId.slot];
		return obj->id == objectId.id ? obj->data : 0;
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