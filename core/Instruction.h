#pragma once
#include "InstructionSet.h"
#include <stdint.h>
#include <vector>
#include <string>

struct Instruction
{
	EOpcode kind;
	std::vector<uint8_t> code;
	std::string codeStr;
};
