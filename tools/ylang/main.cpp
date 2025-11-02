#include "ylang.h"
#include <iostream>
#include <format>
#include <filesystem>
using namespace std;

int main(int argc, const char** argv)
{
	if(argc < 2)
	{
		string toolName = filesystem::path{argv[0]}.filename().string();

		cout << format("{} <source file>\n", toolName);
		cout << "\n";
		cout << format("ex) {} perfect.y\n", toolName);
		return 1;
	}

	ylang y;
	return y.Run(argv[1]) == false;
}
