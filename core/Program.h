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


struct Class
{
	std::string name;
	std::unordered_map<std::string, int> _fieldMap;
	std::vector<Symbol> _fields;
	Bytecode _initer;
	Bytecode _ctor;
	std::unordered_map<std::string, Bytecode> _funcTable;
};


struct Program
{
	std::unordered_map<std::string, Class> _classTable;
	std::unordered_map<std::string, int> _moduleTable;

	std::vector<Constant> _consts;
	Bytecode _mainCode;
};
