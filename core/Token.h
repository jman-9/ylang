#pragma once
#include <stdint.h>
#include <string>

enum class EToken
{
	None,

	Space,
	Tab,

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
	And,
	Or,
	Greater,
	Less,
	GreaterEqual,
	LessEqual,
	Equal,
	NotEqual,
	Amp,
	Pipe,
	Caret,
	LShift,
	RShift,
	Not,
	Assign,
	PlusAssign,
	MinusAssign,
	MulAssign,
	DivAssign,
	ModAssign,
	AndAssign,
	OrAssign,
	XorAssign,
	LShiftAssign,
	RShiftAssign,
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


	static bool IsWhiteSpace(EToken tok);

protected:
};
