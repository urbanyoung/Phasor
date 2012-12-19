#include "Objects.h"
#include "../Addresses.h"

namespace halo { namespace objects {
	s_halo_object* GetObjectAddress(DWORD objectId)
	{
		// Make sure request data is valid
		if (objectId == -1)	return 0;
		int mode = 3;

		WORD hi = (objectId >> 16) & 0xFFFF;
		WORD lo = (objectId & 0xFFFF);

		LPBYTE lpObjectList = (LPBYTE)*(DWORD*)UlongToPtr(ADDR_OBJECTBASE);

		if (lo < (*(WORD*)(lpObjectList + 0x20)))
		{
			LPBYTE objPtr = (LPBYTE)ULongToPtr(*(DWORD*)(lpObjectList + 0x34) + (*(WORD*)(lpObjectList + 0x22) * lo));

			if (!*(WORD*)objPtr) // does obj still exist?
				return 0;

			LPBYTE finalPtr = 0;
			if (!hi)
				finalPtr = objPtr;
			else if (hi == *(WORD*)objPtr)
				finalPtr = objPtr;

			if (!finalPtr)
				return 0;

			if (mode < (1 << finalPtr[2]))
				return (s_halo_object*)*(DWORD*)UlongToPtr(finalPtr + 8);
		}

		return 0;
	}
}}