#pragma once
/*#inclide <
#ifndef _WIN32
typedef unsigned char	BYTE;
typedef int				BOOL;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;
typedef BYTE*			LPBYTE;
typedef void*			LPVOID;
typedef void*			HANDLE;
#define NULL 0

#else 
#include <WinDef.h>
#endif
*/
#include <windows.h>
#define NELEMS(x)  (sizeof(x) / sizeof(x[0]))