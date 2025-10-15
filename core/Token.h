#pragma once
#include <stdint.h>
#include <string>

enum class EToken
{
	Comma,
	Semicolon,
	Colon,
	Dot,
	Quot,
	DblQuot,
	LParen,
	RParen,
	LBrace,
	RBrace,
	LBracket,
	RBracket,
	Plus,
	Minus,
	Star,
	Slash,
	Percent,
	PlusPlus,
	MinusMinus,
	Greater,
	Less,
	GreaterEqual,
	LessEqual,
	Equal,
	NotEqual,
	Amp,
	Pipe,
	LShift,
	Rshift,
	Not,
	Assign,
	PlusAssign,
	MinusAssign,
	MulAssign,
	ModAssign,
	Id,
	Num,
};


struct Token
{
	Token();
	virtual ~Token();

	EToken type;
	uint32_t line;
	uint32_t col;
	std::string val;

protected:

};
