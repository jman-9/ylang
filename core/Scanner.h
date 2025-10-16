#pragma once
#include "Token.h"
#include <stdint.h>
#include <string>
#include <map>


class Scanner
{
public:
	Scanner();
	virtual ~Scanner();

	bool Scan(const std::string& code);

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

	bool AdvanceNewLine(const std::string& code, int& inoutPos);
};
