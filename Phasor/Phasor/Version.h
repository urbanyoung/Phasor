#pragma once

#define PHASOR_MAJOR_VERSION			0x01001059
#define PHASOR_MAJOR_VERSION_STR		L"01.00.10.059"
#define PHASOR_INTERNAL_VERSION			0x029
#ifdef PHASOR_PC
#define PHASOR_HALO_BUILD				L"PC"
#elif PHASOR_CE
#define PHASOR_HALO_BUILD				L"CE"
#else
static_assert(false, "define PHASOR_PC or PHASOR_CE");
#endif