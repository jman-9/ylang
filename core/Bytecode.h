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

	Type _type;

	int64_t _int;
	std::string _str;
	double _float;
};

struct Bytecode
{
	std::vector<Constant> _consts;
	std::vector<Instruction> _code;
};
