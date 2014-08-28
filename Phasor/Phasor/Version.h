#pragma once

#define PHASOR_MAJOR_VERSION			0x020020000
#define PHASOR_MAJOR_VERSION_STR		L"02.02.29.08"
#define PHASOR_INTERNAL_VERSION			0x01
#ifdef PHASOR_PC
#define PHASOR_HALO_BUILD				L"PC"
#define PHASOR_HALO_BUILDA				L"PC"
#elif PHASOR_CE
#define PHASOR_HALO_BUILD				L"CE"
#define PHASOR_HALO_BUILDA				L"CE"
#else
static_assert(false, "define PHASOR_PC or PHASOR_CE");
#endif