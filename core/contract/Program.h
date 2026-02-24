#pragma once
#include "Bytecode.h"
#include <vector>
#include <string>
#include <unordered_map>


struct Closure
{
	std::string _uniqueName;
	std::string _realName;
	Bytecode _code;
};

struct Constant
{
	enum Type
	{
		NONE,

		INT,
		FLOAT,
		STR,
		CLOSURE,
		GLOBAL_FN,
	};

	Type _type;

	int64_t _int;
	std::string _str;
	double _float;
	Closure _closure;
};


struct Class
{
	std::string name;
	std::unordered_map<std::string, int> _fieldMap;
	std::vector<Symbol> _fields;
	Bytecode _initer;
	Bytecode _ctor;
	std::unordered_map<std::string, int> _funcMap;
	std::vector<Bytecode> _funcs;
	std::vector<std::string> _funcNames;
};


enum class EGlobalSymbol
{
	None,

	Var,
	Fn,
	Cls,
};
struct GlobalSymbol
{
	EGlobalSymbol kind = EGlobalSymbol::None;

	std::string name = "";

	// var
	uint32_t idx = 0;

	// fn
	uint32_t seg = 0;
	uint32_t pos = 0;
	uint32_t prms = 0;

	// cls
	Class cls;
};


struct Program
{
	std::string _name;
	std::string _path;

	std::unordered_map<std::string, Class> _classTable;
	std::unordered_map<std::string, int> _moduleTable;
	std::unordered_map<std::string, Program> _programTable;

	std::vector<Constant> _consts;
	Bytecode _mainCode;

	std::unordered_map<std::string, GlobalSymbol> _globalTable;
};
