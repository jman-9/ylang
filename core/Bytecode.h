#pragma once
#include "Instruction.h"
#include <cstdint>
#include <string>
#include <vector>

struct Constant
{
	enum Type
	{
		NONE,

		INT,
		FLOAT,
		STR,
	};

	Type type;

	int64_t _int;
	std::string str;
	double _float;
};

struct Bytecode
{
	std::vector<Constant> _consts;
	std::vector<Instruction> _code;
};
