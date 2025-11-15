#pragma once
#include <stdint.h>
#include <string>


enum class EErr
{
	NoError,
	Error,
	UnknownCharacter,
	UnexpectedCharacter,
	NewLineInString,
	UnexpectedEof,
	UnrecognizedCharacterEscapeSequence,
	UnsupportedCharacterEscapeSequence,
	SyntaxError,
	Missing,
	LValueError,
	ExpectedExpression,
	Expected,
	AlreadyExisting,
};

struct Error
{
	uint32_t line = 0;
	EErr code = EErr::NoError;
	std::string msg = "";

	bool IsNoError() const;
	bool IsIncompleteError() const;
	static bool IsIncompleteError(EErr e);
};

namespace ErrorBuilder
{
	Error NoError();

	Error Default(uint32_t line, const std::string& s);

	Error UnknownCharacter(uint32_t line, char c);
	Error UnexpectedCharacter(uint32_t line, char c);
	Error NewLineInString(uint32_t line);
	Error UnexpectedEof(uint32_t line);
	Error UnrecognizedCharacterEscapeSequence(uint32_t line, char c);
	Error UnsupportedCharacterEscapeSequence(uint32_t line, char c);
	Error SyntaxError(uint32_t line, const std::string& s);
	Error SyntaxError(uint32_t line, char c);
	Error Missing(uint32_t line, char c);
	Error LValueError(uint32_t line, const std::string& s);
	Error ExpectedExpression(uint32_t line, const std::string& s);
	Error Expected(uint32_t line, const std::string& s);
	Error AlreadyExisting(uint32_t line, const std::string& s);
}
