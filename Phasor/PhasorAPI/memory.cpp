#include "memory.h"

using namespace Common;
using namespace Manager;

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

void l_readbit(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{

}
void l_readbyte(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{

}
void l_readword(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{

}
void l_readdword(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{

}
void l_readfloat(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{

}
void l_writebit(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{

}
void l_writebyte(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{

}
void l_writeword(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{

}
void l_writedword(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{

}
void l_writefloat(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{

}
