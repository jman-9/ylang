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
	Str,
	RawStr,
};


struct Token
{
	EToken kind		= EToken::None;
	uint32_t line	= 0;
	std::string val	= "";

	bool IsNull() const;

	static const Token& Null();
	static bool IsWhiteSpace(EToken tok);

protected:
};
