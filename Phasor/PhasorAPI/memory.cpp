#include "memory.h"

using namespace Common;

// Attempt to read data at the specified address
template <class T> bool read_data(LPBYTE destAddress, T & data)
{
	bool success = false;
	DWORD oldProtect = 0;
	if (VirtualProtect(UlongToPtr(destAddress), sizeof(T), PAGE_EXECUTE_READ, &oldProtect))
	{
		data = *(T*)(destAddress);
		VirtualProtect(UlongToPtr(destAddress), sizeof(T), oldProtect, &oldProtect);
		success = true;
	}

	return success;	
}

// Attempt to write data at the specified address
template <class T> bool writeData(LPBYTE destAddress, T data)
{
	bool success = false;
	DWORD oldProtect = 0;
	if (VirtualProtect(UlongToPtr(destAddress), sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		*(T*)(destAddress) = data;
		VirtualProtect(UlongToPtr(destAddress), sizeof(T), oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), UlongToPtr(destAddress), sizeof(T)); 
		success = true;
	}
	return success;
}

void l_readbit(Object::unique_deque& args, Object::unique_list& results)
{

}
void l_readbyte(Object::unique_deque& args, Object::unique_list& results)
{

}
void l_readword(Object::unique_deque& args, Object::unique_list& results)
{

}
void l_readdword(Object::unique_deque& args, Object::unique_list& results)
{

}
void l_readfloat(Object::unique_deque& args, Object::unique_list& results)
{

}
void l_writebit(Object::unique_deque& args, Object::unique_list& results)
{

}
void l_writebyte(Object::unique_deque& args, Object::unique_list& results)
{

}
void l_writeword(Object::unique_deque& args, Object::unique_list& results)
{

}
void l_writedword(Object::unique_deque& args, Object::unique_list& results)
{

}
void l_writefloat(Object::unique_deque& args, Object::unique_list& results)
{

}
