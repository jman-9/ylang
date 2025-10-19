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

}
