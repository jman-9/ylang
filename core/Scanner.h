#pragma once
#include "Token.h"
#include "Error.h"
#include <stdint.h>
#include <string>
#include <map>
#include <vector>


class Scanner
{
public:
	Scanner();
	virtual ~Scanner();

	bool Scan(const std::string& code);

	std::vector<Token> _tokens;
	std::vector<Error> _errors;

protected:
	struct TransTbl
	{
		std::string tokStr;
		EToken tok;
		std::map<char, TransTbl> next;
	};

	TransTbl _transTbl;

	void Init();
	void RemoveComments(std::string& retCode);

	uint32_t AdvanceNewLine(const std::string& code, int pos);

	uint32_t AdvanceRawString(const std::string& code, int start, int& retLines, Error& retError);
	uint32_t AdvanceString(const std::string& code, int start, Error& retError);
	uint32_t AdvanceNumber(const std::string& code, int start, Error& retError);
	uint32_t AdvanceId(const std::string& code, int start, Error& retError);
};
