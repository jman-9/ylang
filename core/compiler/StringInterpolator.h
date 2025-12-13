#pragma once
#include "Token.h"
#include "contract/Error.h"
#include <vector>



namespace ycom
{

class StringInterpolator
{
public:
	struct Result
	{
		std::vector<Error> errs;
		std::vector<Token> res;
	};

	Result Interpolate(const Token& tokStr);

protected:
	Error ReplaceAllEscapeChars(std::string& inoutSrc, int line);
};

}
