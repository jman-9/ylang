#include "StringInterpolator.h"
#include "Scanner.h"
#include <string>
using namespace std;


bool StringInterpolator::ReplaceAllEscapeChars(string& inoutSrc)
{
	string after = "";
	for(int i = 0; i < inoutSrc.size(); )
	{
		char c = inoutSrc[i];
		if(c == '\\')
		{
			if(++i >= inoutSrc.size())
			{//TODO error
				throw 'n';
				//_errors.push_back(ErrorBuilder::UnexpectedEof(t.line));
				return false;
			}
			c = inoutSrc[i];
			if(c == '\'' || c == '"' || c == '?' || c == '\\')
			{//noop
			}
			else if(c == 'a')
			{
				c = 0x07;
			}
			else if(c == 'b')
			{
				c = 0x08;
			}
			else if(c == 'f')
			{
				c = 0x0C;
			}
			else if(c == 'n')
			{
				c = 0x0A;
			}
			else if(c == 'r')
			{
				c = 0x0D;
			}
			else if(c == 't')
			{
				c = 0x09;
			}
			else if(c == 'v')
			{
				c = 0x0B;
			}
			else if(c == 'x')
			{//hexa
				//TODO error
				//_errors.push_back(ErrorBuilder::UnsupportedCharacterEscapeSequence(t.line, c));
				return false;
			}
			else if('0' <= c && c <= '7')
			{//octa
			 //TODO error
				//_errors.push_back(ErrorBuilder::UnsupportedCharacterEscapeSequence(t.line, c));
				return false;
			}
			else if(c == 'u' || c == 'U')
			{//unicode
			 //TODO error
				//_errors.push_back(ErrorBuilder::UnsupportedCharacterEscapeSequence(t.line, c));
				return false;
			}
			else
			{//TODO error
				//_errors.push_back(ErrorBuilder::UnrecognizedCharacterEscapeSequence(t.line, c));
				return false;
			}
		}

		after.push_back(c);
		i++;
	}
#ifdef DEBUG_OUT
	cout << "before: " << src << endl;
	cout << "after: " << after << endl;
#endif
	inoutSrc = after;
	return true;
}


std::vector<Token> StringInterpolator::Interpolate(const Token& tokStr)
{
	if(tokStr != EToken::Str)
		return {};

	std::vector<Token> result;

	size_t i, j;
	string s = tokStr.val;
	for( ; ; )
	{
		for(i=0; i<s.size(); i++)
		{
			if(s[i] == '}')
			{
				if(s[i+1] != '}')
				{//TODO error
					throw 'n';
				}

				s.erase(i+1, 1);
				continue;
			}

			if(s[i] != '{')
				continue;

			if(s[i+1] == '{')
			{
				s.erase(i+1, 1);
				continue;
			}

			for(j=i+1; j<s.size(); j++)
			{
				if(s[j] == '}') break;
			}
			if(j>=s.size())
			{//todo error
				throw 'n';
			}

			string f = s.substr(i+1, j-i-1);
			Scanner sc;
			sc.Scan(f, tokStr.line);
			if(!sc._errors.empty())
			{//TODO error
				throw 'n';
			}

			Token frontStrToken{ EToken::Str, tokStr.line, s.substr(0, i) };
			if(!ReplaceAllEscapeChars(frontStrToken.val))
			{//TODO
				throw 'n';
			}

			result.push_back(frontStrToken);
			if(!sc._tokens.empty())
			{
				result.push_back({ EToken::Plus, tokStr.line, "+" });
				result.push_back({ EToken::LParen, tokStr.line, "(" });
				result.insert(result.end(), sc._tokens.begin(), sc._tokens.end());
				result.push_back({ EToken::RParen, tokStr.line, ")" });
			}
			result.push_back({ EToken::Plus, tokStr.line, "+" });

			s = s.substr(j+1);
			i = 0;
			break;
		}
		if(i >= s.size())
		{
			result.push_back({ EToken::Str, tokStr.line, s });
			if(!ReplaceAllEscapeChars(result.back().val))
			{//TODO
				throw 'n';
			}
			break;
		}
	}

	return result;
}
