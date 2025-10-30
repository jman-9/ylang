#include "Token.h"
#include <unordered_map>
using namespace std;


static unordered_map<EToken, string_view> s_tokenStrMap;

static bool Init()
{
	s_tokenStrMap[EToken::Comma] = ",";
	s_tokenStrMap[EToken::Semicolon] = ";";
	s_tokenStrMap[EToken::Colon] = ":";
	s_tokenStrMap[EToken::Dot] = ".";
	s_tokenStrMap[EToken::LParen] = "(";
	s_tokenStrMap[EToken::RParen] = ")";
	s_tokenStrMap[EToken::LBrace] = "{";
	s_tokenStrMap[EToken::RBrace] = "}";
	s_tokenStrMap[EToken::LBracket] = "[";
	s_tokenStrMap[EToken::RBracket] = "]";
	s_tokenStrMap[EToken::Plus] = "+";
	s_tokenStrMap[EToken::Minus] = "-";
	s_tokenStrMap[EToken::Star] = "*";
	s_tokenStrMap[EToken::Slash] = "/";
	s_tokenStrMap[EToken::Percent] = "%";
	s_tokenStrMap[EToken::PlusPlus] = "++";
	s_tokenStrMap[EToken::MinusMinus] = "--";
	s_tokenStrMap[EToken::And] = "&&";
	s_tokenStrMap[EToken::Or] = "||";
	s_tokenStrMap[EToken::Greater] = ">";
	s_tokenStrMap[EToken::Less] = "<";
	s_tokenStrMap[EToken::GreaterEqual] = ">=";
	s_tokenStrMap[EToken::LessEqual] = "<=";
	s_tokenStrMap[EToken::Equal] = "==";
	s_tokenStrMap[EToken::NotEqual] = "!=";
	s_tokenStrMap[EToken::Amp] = "&";
	s_tokenStrMap[EToken::Pipe] = "|";
	s_tokenStrMap[EToken::Tilde] = "~";
	s_tokenStrMap[EToken::Caret] = "^";
	s_tokenStrMap[EToken::LShift] = "<<";
	s_tokenStrMap[EToken::RShift] = ">>";
	s_tokenStrMap[EToken::Not] = "!";
	s_tokenStrMap[EToken::Assign] = "=";
	s_tokenStrMap[EToken::PlusAssign] = "+=";
	s_tokenStrMap[EToken::MinusAssign] = "-=";
	s_tokenStrMap[EToken::MulAssign] = "*=";
	s_tokenStrMap[EToken::DivAssign] = "/=";
	s_tokenStrMap[EToken::ModAssign] = "%=";
	s_tokenStrMap[EToken::AndAssign] = "&=";
	s_tokenStrMap[EToken::OrAssign] = "|=";
	s_tokenStrMap[EToken::XorAssign] = "^=";
	s_tokenStrMap[EToken::LShiftAssign] = "<<=";
	s_tokenStrMap[EToken::RShiftAssign] = ">>=";
	s_tokenStrMap[EToken::If] = "if";
	s_tokenStrMap[EToken::Else] = "else";
	s_tokenStrMap[EToken::For] = "for";
	s_tokenStrMap[EToken::Break] = "break";
	s_tokenStrMap[EToken::Continue] = "continue";
	s_tokenStrMap[EToken::Fn] = "fn";
	s_tokenStrMap[EToken::Return] = "return";
	s_tokenStrMap[EToken::Invoke] = "invoke";
	s_tokenStrMap[EToken::Index] = "???";
	s_tokenStrMap[EToken::UnaryPlus] = "+";
	s_tokenStrMap[EToken::UnaryMinus] = "-";
	return true;
}
static bool init = Init();


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


string_view Token::TokenString(EToken tok)
{
	auto found = s_tokenStrMap.find(tok);
	return found != s_tokenStrMap.end() ? found->second : "";
}
