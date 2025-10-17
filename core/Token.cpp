#include "Token.h"
using namespace std;


bool Token::IsNull() const
{
	return type == EToken::None;
}

const Token& Token::Null()
{
	static Token s_nullTok = { EToken::None, 0, string("") };
	return s_nullTok;
}

bool Token::IsWhiteSpace(EToken tok)
{
	return tok == EToken::Space || tok == EToken::Tab;
}
