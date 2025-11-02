#include "ylang.h"
#include <iostream>
#include <format>
#include <filesystem>
using namespace std;

int main(int argc, const char** argv)
{

	std::filesystem::path cwd = std::filesystem::current_path();
	std::cout << "Current working directory: " << cwd << '\n';

	if(argc < 2)
	{
		string toolName = filesystem::path{argv[0]}.filename().string();

		cout << format("{} <source file>\n", toolName);
		cout << "\n";
		cout << format("ex) {} perfect.y\n", toolName);
		return 1;
	}

	ylang y;
	y.Run(argv[1]);
}
