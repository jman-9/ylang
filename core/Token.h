#pragma once
#include <stdint.h>
#include <string>

enum class EToken : uint8_t
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
	Tilde,
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

	If,
	Else,
	For,
	Break,
	Continue,
	Fn,
	Return,

	Invoke,
	Index,
	UnaryPlus,
	UnaryMinus,
};


struct Token
{
	EToken kind		= EToken::None;
	uint32_t line	= 0;
	std::string val	= "";

	bool IsNull() const;
	bool IsWhiteSpace() const;
	bool IsLiteral() const;
	bool IsPrefixUnary() const;
	bool IsAssign() const;
	bool Is(EToken tok) const;
	bool operator==(EToken rhs) const;

	static const Token& Null();
	static bool IsWhiteSpace(EToken tok);
	static bool IsLiteral(EToken tok);
	static bool IsPrefixUnary(EToken tok);
	static bool IsAssign(EToken tok);
	static bool IsWhiteSpace(const Token& tok);
	static bool IsLiteral(const Token& tok);
	static bool IsPrefixUnary(const Token& tok);
	static bool IsAssign(const Token& tok);

	static std::string_view TokenString(EToken tok);
};
