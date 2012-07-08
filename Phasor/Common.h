#pragma once

#include <windows.h>
#include <string>

namespace common 
{
	std::string NarrowString(std::wstring);
	std::wstring WidenString(std::string);
}
