#include "StringInterpolator.h"
#include "Scanner.h"
#include <string>
using namespace std;


namespace ycom
{

Error StringInterpolator::ReplaceAllEscapeChars(std::string& inoutSrc, int line)
{
	string after = "";
	for(int i = 0; i < inoutSrc.size(); )
	{
		char c = inoutSrc[i];
		if(c == '\\')
		{
			if(++i >= inoutSrc.size())
			{//TODO error
				return ErrorBuilder::UnexpectedEof(line);
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
			{
				int first = ++i;
				for( ; i - first < 2; i++)
				{
					if('A' <= inoutSrc[i] && inoutSrc[i] <= 'F') continue;
					if('a' <= inoutSrc[i] && inoutSrc[i] <= 'f') continue;
					if('0' <= inoutSrc[i] && inoutSrc[i] <= '9') continue;
					break;
				}
				if(i - first == 0)
				{
					return ErrorBuilder::Default(line, "integer literals must have at least one digit");
				}
				string sub = inoutSrc.substr(first, i - first);
				c = (char)(stoi(sub, nullptr, 16) & 0xFF);
				i--;
			}
			else if('0' <= c && c <= '7')
			{
				int first = i++;
				for( ; ('0' <= inoutSrc[i] && inoutSrc[i] <= '7') && (i - first < 3); i++) {}
				string sub = inoutSrc.substr(first, i - first);
				c = (char)(stoi(sub, nullptr, 8) & 0xFF);
				i--;
			}
			else if(c == 'u' || c == 'U')
			{
				return ErrorBuilder::Default(line, "\\u, \\U : currently unsupported");
			}
			else
			{
				return ErrorBuilder::UnexpectedCharacter(line, c);
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
	return {};
}


StringInterpolator::Result StringInterpolator::Interpolate(const Token& tokStr)
{
	if(tokStr != EToken::Str)
		return {};

	Result result;

	size_t i, j;
	string s = tokStr.val;
	for( ; ; )
	{
		for(i=0; i<s.size(); i++)
		{
			if(s[i] == '}')
			{
				if(s[i+1] != '}')
				{
					result.errs.push_back(ErrorBuilder::UnexpectedCharacter(tokStr.line, s[i+1]));
					return result;
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
			{
				result.errs.push_back(ErrorBuilder::UnexpectedEof(tokStr.line));
				return result;
			}

			string f = s.substr(i+1, j-i-1);
			Scanner sc;
			sc.Scan(f, tokStr.line);
			if(!sc._errors.empty())
			{
				result.errs = sc._errors;
				return result;
			}

			Token frontStrToken{ EToken::Str, tokStr.line, s.substr(0, i) };
			auto err = ReplaceAllEscapeChars(frontStrToken.val, tokStr.line);
			if(!err.IsNoError())
			{
				result.errs.push_back(err);
				return result;
			}

			result.res.push_back(frontStrToken);
			if(!sc._tokens.empty())
			{
				result.res.push_back({ EToken::Plus, tokStr.line, "+" });
				result.res.push_back({ EToken::LParen, tokStr.line, "(" });
				result.res.insert(result.res.end(), sc._tokens.begin(), sc._tokens.end());
				result.res.push_back({ EToken::RParen, tokStr.line, ")" });
			}
			result.res.push_back({ EToken::Plus, tokStr.line, "+" });

			s = s.substr(j+1);
			i = 0;
			break;
		}
		if(i >= s.size())
		{
			result.res.push_back({ EToken::Str, tokStr.line, s });
			auto err = ReplaceAllEscapeChars(result.res.back().val, tokStr.line);
			if(!err.IsNoError())
			{
				result.errs.push_back(err);
				return result;
			}
			break;
		}
	}

	return result;
}

}
