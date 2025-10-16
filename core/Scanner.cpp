#include "Scanner.h"
#include <iostream>
using namespace std;

Scanner::Scanner()
{
	Init();
}

Scanner::~Scanner()
{

}

void Scanner::Init()
{
	_transTbl.tokStr = '\0';
	_transTbl.tok = EToken::None;

	_transTbl.next[' '] = { " ", EToken::Space, };
	_transTbl.next['\t'] = { "\t", EToken::Tab, };
	_transTbl.next[','] = { ",", EToken::Comma, };
	_transTbl.next[';'] = { ";", EToken::Semicolon, };
	_transTbl.next[':'] = { ":", EToken::Colon, };
	_transTbl.next['.'] = { ".", EToken::Dot, };
	_transTbl.next['\'']= { "'", EToken::Quot, };
	_transTbl.next['"'] = { "\"", EToken::DblQuot, };
	_transTbl.next['('] = { "(", EToken::LParen, };
	_transTbl.next[')'] = { ")", EToken::RParen, };
	_transTbl.next['{'] = { "{", EToken::LBrace, };
	_transTbl.next['}'] = { "}", EToken::RBrace, };
	_transTbl.next['['] = { "[", EToken::LBracket, };
	_transTbl.next[']'] = { "]", EToken::RBracket, };
	_transTbl.next['+'] = { "+", EToken::Plus, };
	_transTbl.next['-'] = { "-", EToken::Minus, };
	_transTbl.next['*'] = { "*", EToken::Star, };
	_transTbl.next['/'] = { "/", EToken::Slash, };
	_transTbl.next['%'] = { "%", EToken::Percent, };
	_transTbl.next['&'] = { "&", EToken::Amp, };
	_transTbl.next['|'] = { "|", EToken::Pipe, };
	_transTbl.next['^'] = { "^", EToken::Caret, };
	_transTbl.next['!'] = { "!", EToken::Not, };
	_transTbl.next['='] = { "=", EToken::Assign, };
	_transTbl.next['>'] = { ">", EToken::Greater, };
	_transTbl.next['<'] = { "<", EToken::Less, };

	_transTbl.next['+'].next['+'] = { "++", EToken::PlusPlus, };
	_transTbl.next['-'].next['-'] = { "--", EToken::MinusMinus, };
	_transTbl.next['&'].next['&'] = { "&&", EToken::And, };
	_transTbl.next['|'].next['|'] = { "||", EToken::Or, };
	_transTbl.next['>'].next['='] = { ">=", EToken::GreaterEqual, };
	_transTbl.next['<'].next['='] = { "<=", EToken::LessEqual, };
	_transTbl.next['='].next['='] = { "==", EToken::Equal, };
	_transTbl.next['!'].next['='] = { "!=", EToken::NotEqual, };
	_transTbl.next['<'].next['<'] = { "<<", EToken::LShift, };
	_transTbl.next['>'].next['>'] = { ">>", EToken::RShift, };
	_transTbl.next['+'].next['='] = { "+=", EToken::PlusAssign, };
	_transTbl.next['-'].next['='] = { "-=", EToken::MinusAssign, };
	_transTbl.next['*'].next['='] = { "*=", EToken::MulAssign, };
	_transTbl.next['/'].next['='] = { "/=", EToken::DivAssign, };
	_transTbl.next['%'].next['='] = { "%=", EToken::ModAssign, };
	_transTbl.next['&'].next['='] = { "&=", EToken::AndAssign, };
	_transTbl.next['|'].next['='] = { "|=", EToken::OrAssign, };
	_transTbl.next['^'].next['='] = { "^=", EToken::XorAssign, };
	_transTbl.next['<'].next['<'].next['='] = { "<<=", EToken::LShiftAssign, };
	_transTbl.next['>'].next['>'].next['='] = { ">>=", EToken::RShiftAssign, };
}

bool Scanner::Scan(const string& orgCode)
{
	string code = orgCode;
	RemoveComments(code);

	int lineNum = 0;
	int i = 0;
	const TransTbl* accepted = &_transTbl;
	bool found = false;
	for( ; i<code.size(); )
	{
		if(AdvanceNewLine(code, i))
		{
			lineNum++;
			continue;
		}

		auto next = accepted->next.find(code[i]);
		if(next != accepted->next.end())
		{
			accepted = &next->second;
			i++;
			continue;
		}

		if(accepted->tok == EToken::None)
		{	// not found
			// id, number, ...
			continue;
		}

		if(!Token::IsWhiteSpace(accepted->tok))
			cout << "token : " << accepted->tokStr << endl;
		accepted = &_transTbl;
	}

	return false;
}

void Scanner::RemoveComments(std::string& retCode)
{

}

bool Scanner::AdvanceNewLine(const std::string& code, int& inoutPos)
{
	if(inoutPos >= code.size()) return false;

	if(code[inoutPos] == '\n')
	{
		inoutPos++;
		if(inoutPos >= code.size()) return true;

		if(code[inoutPos] == '\r')
		{
			inoutPos++;
			return true;
		}
	}
	else if(code[inoutPos] == '\r')
	{
		inoutPos++;
		if(inoutPos >= code.size()) return true;

		if(code[inoutPos] == '\n')
		{
			inoutPos++;
			return true;
		}
	}

	return false;
}
