#pragma once
#include <string>

enum class EErr
{
	NoError,
	UnknownCharacter,
	UnexpectedCharacter,
	NewLineInString,
	UnexpectedEof,
	UnrecognizedCharacterEscapeSequence,
	UnsupportedCharacterEscapeSequence,
};

struct Error
{
	uint32_t line = 0;
	EErr code = EErr::NoError;
	std::string msg = "";

	bool IsNoError() const;
};

namespace ErrorBuilder
{
	Error NoError();

	Error UnknownCharacter(uint32_t line, char c);
	Error UnexpectedCharacter(uint32_t line, char c);
	Error NewLineInString(uint32_t line);
	Error UnexpectedEof(uint32_t line);
	Error UnrecognizedCharacterEscapeSequence(uint32_t line, char c);
	Error UnsupportedCharacterEscapeSequence(uint32_t line, char c);
}
