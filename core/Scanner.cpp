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

	uint32_t lineNum = 1;
	int i = 0;
	const TransTbl* accepted = &_transTbl;

	for( ; i<code.size(); )
	{
		uint32_t sz = 0;
		uint32_t lines = 0;
		Error err;

		if(sz = AdvanceNewLine(code, i))
		{
			lineNum++;
			i += sz;
			continue;
		}
		if(sz = AdvanceComment(code, i, lines, err))
		{
			if(!err.IsNoError())
			{
				cerr << err.msg << endl;
				err.line = lineNum + lines;
				_errors.push_back(err);
				return false;
			}

			lineNum += lines;
			i += sz;
			continue;
		}

		char cur = code[i];
		auto next = accepted->next.find(cur);
		if(next != accepted->next.end())
		{
			accepted = &next->second;
			i++;
			continue;
		}

		if(Token::IsWhiteSpace(accepted->tok))
		{// no-op
		}
		else if(accepted->tok == EToken::None)
		{// string, num, id
			EToken tok;

			if(!sz && err.IsNoError()) { tok = EToken::RawStr; sz = AdvanceRawString(code, i, lines, err); }
			if(!sz && err.IsNoError()) { tok = EToken::Str; sz = AdvanceString(code, i, err); }
			if(!sz && err.IsNoError()) { tok = EToken::Num; sz = AdvanceNumber(code, i, err); }
			if(!sz && err.IsNoError()) { tok = EToken::Id; sz = AdvanceId(code, i, err); }

			if(sz && err.IsNoError())
			{
				_tokens.push_back({ tok, lineNum, code.substr(i, sz) });
				i += sz;
				lineNum += lines;
			}
			else if(!err.IsNoError())
			{
				cerr << err.msg << endl;
				err.line = lineNum + lines;
				_errors.push_back(err);
				return false;
			}
			else
			{
				cerr << "unknown character error" << endl;
				_errors.push_back( ErrorBuilder::UnknownCharacter(lineNum, cur) );
				return false;
			}
		}
		else
		{
			cout << "token : " << accepted->tokStr << endl;
			_tokens.push_back({ accepted->tok, lineNum, accepted->tokStr });
		}

		accepted = &_transTbl;
	}

	return true;
}


uint32_t Scanner::AdvanceNewLine(const std::string& code, int pos)
{
	if(pos >= code.size()) return 0;

	if(code[pos] == '\n')
	{
		return pos + 1 < code.size() && code[pos+1] == '\r' ? 2 : 1;
	}
	else if(code[pos] == '\r')
	{
		return pos + 1 < code.size() && code[pos+1] == '\n' ? 2 : 1;
	}

	return 0;
}

uint32_t Scanner::AdvanceComment(const std::string& code, int start, uint32_t& retLines, Error& retError)
{
	retError = ErrorBuilder::NoError();

	if(start >= code.size() - 1) return 0;

	static const string commentToken = "//";
	static const string commentBlockStartToken = "/*";
	static const string commentBlockEndToken = "*/";

	bool commentBlock;

	if(code.compare(start, commentBlockStartToken.size(), commentBlockStartToken) == 0)
	{
		commentBlock = true;
	}
	else if(code.compare(start, commentToken.size(), commentToken) == 0)
	{
		commentBlock = false;
	}
	else
	{
		return 0;
	}

	uint32_t end = 0;
	uint32_t len;
	retLines = 0;
	if(!commentBlock)
	{
		end = start + (uint32_t)commentToken.size();
		for( ; end<code.size(); end++)
		{
			if(len = AdvanceNewLine(code, end))
			{
				end += len;
				retLines = 1;
				break;
			}
		}
	}
	else
	{
		end = start + (uint32_t)commentBlockStartToken.size();
		for( ; end<code.size(); )
		{
			if(len = AdvanceNewLine(code, end))
			{
				end += len;
				retLines++;
				continue;
			}
			else if(code.compare(end, commentBlockEndToken.size(), commentBlockEndToken) == 0)
			{
				end += (int)commentBlockEndToken.size();
				break;
			}
			else
			{
				end++;
			}
		}
		if(end >= code.size())
		{
			retError = ErrorBuilder::UnexpectedEof(0);
		}
	}

	cout << "comment: " << code.substr(start, end - start) << endl;
	return end - start;
}


uint32_t Scanner::AdvanceRawString(const std::string& code, int start, uint32_t& retLines, Error& retError)
{
	retError = ErrorBuilder::NoError();

	if(start >= code.size()) return 0;

	string delim;
	if(code.compare(start, 3, "'''") == 0)
	{
		delim = "'''";
	}
	else if(code.compare(start, 3, "\"\"\"") == 0)
	{
		delim = "\"\"\"";
	}
	else
	{
		return 0;
	}

	int i = start + (int)delim.size();
	int end = -1;

	retLines = 0;
	for( ; i < code.size(); )
	{
		if(code.compare(i, delim.size(), delim, 0) == 0)
			break;

		uint32_t sz = AdvanceNewLine(code, i);
		if(sz)
		{
			retLines++;
			i += sz;
		}
		else
		{
			i++;
		}
	}
	if(i >= code.size())
	{
		retError = ErrorBuilder::UnexpectedEof(0);
		return 0;
	}

	end = i + (int)delim.size();

	cout << "raw: " << code.substr(start, end - start) << endl;
	return end - start;
}

uint32_t Scanner::AdvanceString(const std::string& code, int start, Error& retError)
{
	retError = ErrorBuilder::NoError();

	if(start >= code.size()) return 0;

	char delim = code[start];
	if(delim != '"' && delim != '\'')
		return 0;

	int i = start + 1;
	int end = -1;

	for( ; i < code.size(); i++)
	{
		if(AdvanceNewLine(code, i))
		{
			retError = ErrorBuilder::NewLineInString(0);
			return 0;
		}

		if(code[i] == '\\')
		{
			i++;
			continue;
		}

		if(code[i] == delim)
			break;
	}
	if(i >= code.size())
	{
		retError = ErrorBuilder::UnexpectedEof(0);
		return 0;
	}

	end = i;

	cout << "str: " << code.substr(start, end - start + 1) << endl;
	return end - start + 1;
}

uint32_t Scanner::AdvanceNumber(const std::string& code, int start, Error& retError)
{
	retError = ErrorBuilder::NoError();

	if(start >= code.size()) return 0;

	char first = code[start];
	if(!isdigit(first))
		return 0;

	int i = start + 1;
	int end = -1;

	if(i >= code.size())
	{//no-op
	}
	else if(first == '0')
	{
		if(code[i] == 'x' || code[i] == 'X')
		{//hexa
			for(i++; i<code.size(); i++)
			{
				if('A' <= code[i] && code[i] <= 'F') continue;
				if('a' <= code[i] && code[i] <= 'f') continue;
				if('0' <= code[i] && code[i] <= '9') continue;

				if(isalnum(code[i]) || code[i] == '_')
				{
					retError = ErrorBuilder::UnexpectedCharacter(0, code[i]);
					return 0;
				}

				break;
			}
			if(i - start <= 2) return 0;
		}
		else
		{//octa
			for(i++; i<code.size(); i++)
			{
				if('0' <= code[i] && code[i] <= '7') continue;

				if(isalnum(code[i]) || code[i] == '_')
				{
					retError = ErrorBuilder::UnexpectedCharacter(0, code[i]);
					return 0;
				}

				break;
			}
		}
	}
	else
	{//decimal
		for( ; i<code.size(); i++)
		{
			if(isdigit(code[i])) continue;

			if(isalpha(code[i]) || code[i] == '_')
			{
				retError = ErrorBuilder::UnexpectedCharacter(0, code[i]);
				return 0;
			}

			break;
		}
	}

	end = i;

	cout << "num: " << code.substr(start, end - start) << endl;
	return end - start;
}

uint32_t Scanner::AdvanceId(const std::string& code, int start, Error& retError)
{
	retError = ErrorBuilder::NoError();

	if(start >= code.size()) return 0;

	char first = code[start];
	if(!isalpha(first) && first != '_')
		return 0;

	int i = start + 1;
	int end = -1;

	for(; i < code.size(); i++)
	{
		if(isalnum(code[i]) == false && code[i] != '_')
		{
			break;
		}
	}

	end = i;

	cout << "id: " << code.substr(start, end - start) << endl;
	return end - start;
}
