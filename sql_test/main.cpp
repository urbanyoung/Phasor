#include <string>
#include <stdio.h>

int main()
{
	std::string s = "test";
	char* cstr = (char*)s.c_str();
	cstr[0] = 'p';
	printf("String: %s\nc_str: %s\n", s.c_str(), cstr);
	return 0;
}