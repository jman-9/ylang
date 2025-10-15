#pragma once
#include <stdint.h>
#include <string>

class Scanner
{
public:
	Scanner();
	virtual ~Scanner();

	bool Scan(const std::string& code);

protected:
};
