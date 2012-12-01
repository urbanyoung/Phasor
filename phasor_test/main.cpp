#include <string>
#include <vector>
#include <stdio.h>

// Get the substring ending at the next occurrence of c.
// start is the position (inclusive) where to start searching from.
// end is the position after the next occurrence, or npos if none.
std::string GetStringEndingAtNext(const std::string& input, char c, 
	size_t start, size_t& end)
{
	size_t found = input.find_first_of(c, start);
	std::string out = input.substr(start, found - start);
	end = found == input.npos ? input.npos : found + 1;
	printf("string of length %i\n", out.size());
	return out;
}

std::vector<std::string> test(const std::string& in)
{
	std::vector<std::string> out;
	static const std::string tofind = "\"' "; // " ' or space

	size_t curpos = 0;
	while (curpos != in.npos)
	{
		curpos = in.find_first_not_of(' ', curpos);
		if (curpos == in.npos) break;
		size_t nextpos = in.find_first_of(tofind, curpos);
		if (nextpos == in.npos) { // no more matches, copy everything.
			out.push_back(in.substr(curpos, in.npos));
			break;
		}

		char c = in.at(nextpos);
		size_t startfrom = c == ' ' ? curpos : curpos + 1;
		std::string token = GetStringEndingAtNext(in, c, startfrom, curpos);	
		if (token.size()) out.push_back(token);
	}
	return out;
}

int main()
{
	std::string test_str = "   'a'   b  token1 token2 'token 3' \"token 4 'still' token 4\" token5       '";
	std::vector<std::string> out = test(test_str);

	for (size_t x = 0; x < out.size(); x++)
		printf("%s|\n", out[x].c_str());
	return 0;
}