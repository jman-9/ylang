#include "Error.h"
#include <format>
using namespace std;


bool Error::IsNoError() const
{
	return code == EErr::NoError;
}


namespace ErrorBuilder
{

Error NoError()
{
	return Error();
}

Error Default(uint32_t line, const std::string& s)
{
	return { line, EErr::Error, format("{}", s) };
}

Error UnknownCharacter(uint32_t line, char c)
{
	return { line, EErr::UnknownCharacter, format("unknown character '0x{:02X}'", c) };
}
Error UnexpectedCharacter(uint32_t line, char c)
{
	return { line, EErr::UnknownCharacter, format("unexpected character '0x{:02X}'", c) };
}
Error NewLineInString(uint32_t line)
{
	return { line, EErr::NewLineInString, "new line in string" };
}
Error UnexpectedEof(uint32_t line)
{
	return { line, EErr::UnexpectedEof, "unexpected EOF" };
}
Error UnrecognizedCharacterEscapeSequence(uint32_t line, char c)
{
	return { line, EErr::UnrecognizedCharacterEscapeSequence, format("'{}': unrecognized character escape sequence", c) };
}
Error UnsupportedCharacterEscapeSequence(uint32_t line, char c)
{
	return { line, EErr::UnsupportedCharacterEscapeSequence, format("'{}': unsupported character escape sequence", c) };
}

Error SyntaxError(uint32_t line, const string& s)
{
	return { line, EErr::SyntaxError, format("'{}': syntax error", s) };
}

Error SyntaxError(uint32_t line, char c)
{
	return SyntaxError(line, string(1, c));
}

Error Missing(uint32_t line, char c)
{
	return { line, EErr::Missing, format("'{}': missing", c) };
}

Error LValueError(uint32_t line, const std::string& s)
{
	return { line, EErr::LValueError, format("'{}': left operand must be l-value", s) };
}


}
