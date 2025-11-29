#pragma once
#include "Bytecode.h"
#include <vector>
#include <string>
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


struct Struct
{
	std::string name;
	std::vector<Symbol> _fields;
	Bytecode _initer;
	std::unordered_map<std::string, Bytecode> _funcTable;
};


struct Program
{
	std::unordered_map<std::string, Struct> _structTable;
	std::unordered_map<std::string, int> _moduleTable;

	std::vector<Constant> _consts;
	Bytecode _mainCode;
};
