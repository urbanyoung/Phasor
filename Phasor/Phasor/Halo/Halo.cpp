#include "Halo.h"
#include "Addresses.h"

namespace halo
{
	ident make_ident(unsigned long id) {
		ident out;
		out.slot = (unsigned short)(id & 0xffff);
		out.id = (unsigned short)((id & 0xffff0000) >> 16);
		return out;
	}
}