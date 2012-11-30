#pragma once

#ifndef _WIN32
typedef unsigned char	BYTE;
typedef unsigned long	DWORD;

#define NULL 0
#else
#include <windows.h>
#endif
#define NELEMS(x)  (sizeof(x) / sizeof(x[0]))