#pragma once

#include <string>

std::string NarrowString(const std::wstring& wide);
std::wstring WidenString(const std::string& narrow);