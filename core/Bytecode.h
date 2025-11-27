#pragma once
#include "Symbol.h"
#include "Instruction.h"
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

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


struct Struct
{
	std::vector<Symbol> _fields;
	std::vector<Instruction> _initer;
	std::unordered_map<std::string, std::vector<Instruction>> _funcTable;
};
