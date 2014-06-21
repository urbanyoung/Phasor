#include "misc.h"
#include "api_readers.h"
#include "../../Common/MyString.h"
using namespace Common;
using namespace Manager;

void l_getticks(CallHandler& handler, Object::unique_deque&, Object::unique_list& results)
{
	AddResultNumber(GetTickCount(), results);	
}

void l_getrandomnumber(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	DWORD min = ReadNumber<DWORD>(*args[0]);
	DWORD max = ReadNumber<DWORD>(*args[1]);

	DWORD result = (min == max) ? max : min + rand() % (max - min);
	AddResultNumber(result, results);
}