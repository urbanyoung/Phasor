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

struct real_rgb_color
{
	float red;
	float green;
	float blue;
};

struct real_argb_color
{
	float alpha;
	float red;
	float green;
	float blue;
};

/*union*/ struct real_rectangle2d
{
	float x0, x1; // top, bottom
	float y0, y1; // left, right
};