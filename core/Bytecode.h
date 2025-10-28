#pragma once
#include "Instruction.h"
#include <cstdint>
#include <string>
#include <vector>

struct Constant
{
	enum Type
	{
		NUM,
		STR,
		FLOAT,
	};

	Type type;

	int64_t num;
	std::string str;
	double _float;
};

struct Bytecode
{
	std::vector<Constant> _consts;
	std::vector<Instruction> _code;
};
