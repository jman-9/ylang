#pragma once
#include <string>
#include <vector>


enum class ESymbol
{
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
	std::string name;
	ESymbol kind;

	// fn
	size_t pos;
	std::vector<Param> params;
};
