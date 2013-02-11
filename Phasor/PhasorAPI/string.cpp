#include "string.h"
#include "../Common/MyString.h"
#include "api_readers.h"

using namespace Common;
using namespace Manager;

void l_tokenizestring(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	std::string str = ReadRawString(*args[0]);
	std::string delim = ReadRawString(*args[1]);

	std::vector<std::string> tokens = Tokenize<std::string>(str, delim);
	AddResultTable(tokens, results);	
}

void l_tokenizecmdstring(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	std::string str = ReadRawString(*args[0]);
	std::vector<std::string> tokens = TokenizeArgs(str);
	AddResultTable(tokens, results);
}