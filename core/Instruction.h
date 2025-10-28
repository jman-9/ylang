#pragma once
#include "InstructionSet.h"
#include <stdint.h>
#include <vector>
#include <string>

struct Instruction
{
	Opcode kind;
	std::vector<uint8_t> code;
	std::string codeStr;
};
