#pragma once
#include <string>
#include <vector>


enum class ESymbol
{
	None,

	Var,
	Fn,
	Cls,
};

struct Param
{
	std::string name;
};

struct Symbol
{
	std::string name = "";
	ESymbol kind = ESymbol::None;

	// fn
	size_t pos = 0;
	std::vector<Param> params;
};
