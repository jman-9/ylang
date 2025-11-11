#include "Scanner.h"
#include <iostream>
using namespace std;


//#define DEBUG_OUT


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
	_transTbl.next['~'] = { "~", EToken::Tilde, };
	_transTbl.next['^'] = { "^", EToken::Caret, };
	_transTbl.next['!'] = { "!", EToken::Not, };
	_transTbl.next['='] = { "=", EToken::Assign, };
	_transTbl.next['>'] = { ">", EToken::Greater, };
	_transTbl.next['<'] = { "<", EToken::Less, };

	_transTbl.next['.'] = { ".", EToken::Dot, };
	_transTbl.next['.'].next['0'] = { "", EToken::None, };
	_transTbl.next['.'].next['1'] = { "", EToken::None, };
	_transTbl.next['.'].next['2'] = { "", EToken::None, };
	_transTbl.next['.'].next['3'] = { "", EToken::None, };
	_transTbl.next['.'].next['4'] = { "", EToken::None, };
	_transTbl.next['.'].next['5'] = { "", EToken::None, };
	_transTbl.next['.'].next['6'] = { "", EToken::None, };
	_transTbl.next['.'].next['7'] = { "", EToken::None, };
	_transTbl.next['.'].next['8'] = { "", EToken::None, };
	_transTbl.next['.'].next['9'] = { "", EToken::None, };

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


void Scanner::ProcessIdKeywords()
{
	for(auto& t : _tokens)
	{
		if(t.kind != EToken::Id) continue;

		if(0) {}
		else if(t.val == "if") t.kind = EToken::If;
		else if(t.val == "else") t.kind = EToken::Else;
		else if(t.val == "for") t.kind = EToken::For;
		else if(t.val == "break") t.kind = EToken::Break;
		else if(t.val == "continue") t.kind = EToken::Continue;
		else if(t.val == "fn") t.kind = EToken::Fn;
		else if(t.val == "return") t.kind = EToken::Return;
	}
}

bool Scanner::Scan(const std::string& originCode, int lineStartNum /* = 1 */)
{
	string code = originCode;

	uint32_t lineNum = lineStartNum;
	int i = 0;
	int off = 0;
	uint32_t acceptedLine = 0;
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
				#ifdef DEBUG_OUT
					cerr << err.msg << endl;
				#endif
				err.line = lineNum + lines;
				_errors.push_back(err);
				return false;
			}

			lineNum += lines;
			i += sz;
			continue;
		}

		char cur = code[i+off];
		auto next = accepted->next.find(cur);
		if(next != accepted->next.end())
		{
			accepted = &next->second;
			if(accepted->tok == EToken::None)
			{
				off = 0;
			}
			else
			{
				acceptedLine = lineNum;
				off++;
				continue;
			}
		}

		if(Token::IsWhiteSpace(accepted->tok))
		{// no-op
		}
		else if(accepted->tok == EToken::None)
		{// string, num, id
			EToken tok;
			string parsed = "";

			if(!sz && err.IsNoError()) { tok = EToken::RawStr; sz = AdvanceRawString(code, i, parsed, lines, err); }
			if(!sz && err.IsNoError()) { tok = EToken::Str; sz = AdvanceString(code, i, parsed, err); }
			if(!sz && err.IsNoError()) { tok = EToken::Float; sz = AdvanceFloating(code, i, err); }
			if(!sz && err.IsNoError()) { tok = EToken::Int; sz = AdvanceInteger(code, i, err); }
			if(!sz && err.IsNoError()) { tok = EToken::Id; sz = AdvanceId(code, i, err); }

			if(sz && err.IsNoError())
			{
				if(parsed.empty()) parsed = code.substr(i, sz);
				_tokens.push_back({ tok, lineNum, parsed });

				i += sz;
				lineNum += lines;
			}
			else if(!err.IsNoError())
			{
				#ifdef DEBUG_OUT
					cerr << err.msg << endl;
				#endif
				err.line = lineNum + lines;
				_errors.push_back(err);
				return false;
			}
			else
			{
				#ifdef DEBUG_OUT
					cerr << "unknown character error" << endl;
				#endif
				_errors.push_back( ErrorBuilder::UnknownCharacter(lineNum, cur) );
				return false;
			}
		}
		else
		{
			#ifdef DEBUG_OUT
				cout << "token : " << accepted->tokStr << endl;
			#endif
			_tokens.push_back({ accepted->tok, acceptedLine, accepted->tokStr });
		}

		accepted = &_transTbl;
		acceptedLine = 0;
		i += off;
		off = 0;
	}

	if(accepted != &_transTbl && !Token::IsWhiteSpace(accepted->tok))
		_tokens.push_back({ accepted->tok, acceptedLine, accepted->tokStr });

	ProcessIdKeywords();
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

	#ifdef DEBUG_OUT
		cout << "comment: " << code.substr(start, end - start) << endl;
	#endif
	return end - start;
}


uint32_t Scanner::AdvanceRawString(const std::string& code, int start, std::string& retStr, uint32_t& retLines, Error& retError)
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

	retStr = code.substr(start+delim.size(), i - (start+delim.size()));
	end = i + (int)delim.size();

	#ifdef DEBUG_OUT
		cout << "raw: " << retStr << endl;
	#endif
	return end - start;
}

uint32_t Scanner::AdvanceString(const std::string& code, int start, std::string& retStr, Error& retError)
{
	retError = ErrorBuilder::NoError();

	if(start >= code.size()) return 0;

	char delim = code[start];
	if(delim != '"' && delim != '\'')
		return 0;

	int end = start + 1;
	for( ; end < code.size(); end++)
	{
		if(AdvanceNewLine(code, end))
		{
			retError = ErrorBuilder::NewLineInString(0);
			return 0;
		}

		if(code[end] == '\\')
		{
			end++;
			continue;
		}

		if(code[end] == delim)
			break;
	}
	if(end >= code.size())
	{
		retError = ErrorBuilder::UnexpectedEof(0);
		return 0;
	}

	retStr = code.substr(start+1, end - (start+1));

	#ifdef DEBUG_OUT
		cout << "str: " << retStr << endl;
	#endif
	return end - start + 1;
}

uint32_t Scanner::AdvanceFloating(const std::string& code, int start, Error& retError)
{
	retError = ErrorBuilder::NoError();

	if(start >= code.size()) return 0;

	char first = code[start];
	if(!isdigit(first) && first != '.')
		return 0;

	enum class EFloat
	{
		NONE,
		FLOAT,
		EXP,
	};

	int i = start + 1;
	int end = -1;
	EFloat type = EFloat::NONE;

	if(i >= code.size())
	{//not float
		return 0;
	}
	else if(isdigit(first))
	{
		for(; i<code.size(); i++)
		{
			if(isdigit(code[i]))
				continue;

			if(code[i] == '.')
			{
				i++;
				type = EFloat::FLOAT;
				break;
			}
			else if(code[i] == 'e' || code[i] == 'E')
			{
				i++;
				type = EFloat::EXP;
				break;
			}
			else
			{//integer
				return 0;
			}
		}
	}
	else
	{
		if(isdigit(code[i]))
			type = EFloat::FLOAT;
	}

	if(type == EFloat::NONE)
	{
		return 0;
	}

	if(type == EFloat::FLOAT)
	{
		for(; i<code.size(); i++)
		{
			if(!isdigit(code[i]))
				break;
		}

		if(i < code.size() && (code[i] == 'e' || code[i] == 'E'))
		{
			i++;
			type = EFloat::EXP;
		}
	}

	if(type == EFloat::EXP)
	{
		if(i >= code.size())
		{//TODO err
			throw 'n';
		}
		else if(!isdigit(code[i]) && code[i] != '+' && code[i] != '-')
		{//TODO err
			throw 'n';
		}

		if(code[i] == '+' || code[i] == '-')
		{
			i++;
		}

		if(i >= code.size())
		{//TODO err
			throw 'n';
		}
		else if(!isdigit(code[i]))
		{//TODO err
			throw 'n';
		}

		for(; i<code.size(); i++)
		{
			if(!isdigit(code[i]))
				break;
		}
	}

	end = i;

#ifdef DEBUG_OUT
	cout << "float: " << code.substr(start, end - start) << endl;
#endif
	return end - start;
}

uint32_t Scanner::AdvanceInteger(const std::string& code, int start, Error& retError)
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
			for(; i<code.size(); i++)
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

	#ifdef DEBUG_OUT
		cout << "num: " << code.substr(start, end - start) << endl;
	#endif
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

	#ifdef DEBUG_OUT
		cout << "id: " << code.substr(start, end - start) << endl;
	#endif
	return end - start;
}
