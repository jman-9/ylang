#include "Token.h"
using namespace std;


bool Token::IsNull() const
{
	return kind == EToken::None;
}

bool Token::IsWhiteSpace() const
{
	return Token::IsWhiteSpace(kind);
}
bool Token::IsLiteral() const
{
	return Token::IsLiteral(kind);
}
bool Token::IsPrefixUnary() const
{
	return Token::IsPrefixUnary(kind);
}

bool Token::IsAssign() const
{
	return Token::IsAssign(kind);
}

bool Token::Is(EToken tok) const
{
	return kind == tok;
}

bool Token::operator==(EToken rhs) const
{
	return Is(rhs);
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

bool Token::IsLiteral(EToken tok)
{
	return tok == EToken::Num || tok == EToken::Str || tok == EToken::RawStr;
}

bool Token::IsPrefixUnary(EToken tok)
{
	return tok == EToken::Not || tok == EToken::Tilde || tok == EToken::Plus || tok == EToken::Minus || tok == EToken::UnaryPlus || tok == EToken::UnaryMinus;
}

bool Token::IsAssign(EToken tok)
{
	return EToken::Assign <= tok && tok <= EToken::RShiftAssign;
}

bool Token::IsWhiteSpace(const Token& tok)
{
	return IsWhiteSpace(tok.kind);
}

bool Token::IsLiteral(const Token& tok)
{
	return IsLiteral(tok.kind);
}

bool Token::IsPrefixUnary(const Token& tok)
{
	return IsPrefixUnary(tok.kind);
}

bool Token::IsAssign(const Token& tok)
{
	return IsAssign(tok.kind);
}
