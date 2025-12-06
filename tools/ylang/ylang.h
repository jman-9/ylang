#pragma once
#include "contract/Program.h"
#include <string>

class ylang
{
public:
	bool RunCode(const std::string& src);
	bool RunFile(const std::string& srcPath);

	bool StartRepl();

protected:
};
