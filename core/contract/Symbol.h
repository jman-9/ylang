#pragma once
#include <string>
#include <vector>


enum class ESymbol
{
	None,

	Var,
	Fn,
	Cls,
	Mod,
	Field,
	Prg,
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
	size_t seg = 0;
	size_t pos = 0;
	std::vector<Param> params;

	bool preRegister = false;

	bool IsNone() const { return kind == ESymbol::None; }
	bool operator== (ESymbol rhs) const { return kind == rhs; }
	bool operator!= (ESymbol rhs) const { return kind != rhs; }
};
