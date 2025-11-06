#include "ylang.h"
#include <iostream>
#include <format>
#include <filesystem>
using namespace std;

int main(int argc, const char** argv)
{
	ylang y;

	if(argc < 2)
	{
		return y.StartRepl() == false;
	}
	else
	{
		if(strcmp(argv[1], "--help") == 0)
		{
			string toolName = filesystem::path{argv[0]}.filename().string();

			cout << format("{} <source file>\n", toolName);
			cout << "\n";
			cout << format("ex) {} perfect.y\n", toolName);
			return 1;
		}
	}

	return y.RunFile(argv[1]) == false;
}
