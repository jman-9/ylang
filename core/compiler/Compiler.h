#pragma once
#include "contract/Error.h"
#include "contract/Program.h"
#include <string>
#include <vector>


namespace ycom
{

class Compiler
{
public:
	std::vector<Error> CompileCode(const std::string& src, Program& retProgram);
	std::vector<Error> CompileFile(const std::string& srcPath, Program& retProgram);
};

}
