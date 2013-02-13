#include "Objects.h"
#include "../Addresses.h"
#include "../../Globals.h"

namespace halo { namespace objects {

	/*! \todo look into expanding object limit.. probably not possible due to clients tho */
	struct s_halo_object_table
	{
		s_table_header header;
		s_halo_object entries[0x800];
	};	

	/*! \todo check this works */
	s_halo_object* GetObjectAddress(ident objectId)
	{
		s_halo_object_table* object_table = *(s_halo_object_table**)ADDR_OBJECTBASE;
		if (objectId.slot >= object_table->header.max_size) return 0;

		s_halo_object* obj = &object_table->entries[objectId.slot];
		return obj->id == objectId.id ? obj : 0;
	}
}}