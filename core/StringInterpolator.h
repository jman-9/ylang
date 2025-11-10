#pragma once
#include "Token.h"
#include <vector>


class StringInterpolator
{
public:
	std::vector<Token> Interpolate(const Token& tokStr);

protected:
	bool ReplaceAllEscapeChars(std::string& inoutSrc);
};
