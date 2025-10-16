#include "Token.h"


Token::Token()
{
}

Token::~Token()
{
}

bool Token::IsWhiteSpace(EToken tok)
{
	return tok == EToken::Space || tok == EToken::Tab;
}
