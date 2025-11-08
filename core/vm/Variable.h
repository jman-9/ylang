#pragma once
#include "../Token.h"
#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>



namespace yvm
{

struct Variable
{
	enum Type
	{
		NONE,

		NUM,
		STR,
		FLOAT,
		OBJECT,
		LIST,
		DICT,
		REF,
	};

	Type type = NONE;

	int64_t num = 0;
	std::string str = "";
	double _float = 0.0;
	void* obj = nullptr;
	std::vector<Variable*>* list;
	std::unordered_map<std::string, Variable*>* dict;
	Variable* ref = nullptr;

	void Clear();
	Variable* Clone();

	bool Assign(EToken op, const Variable& rval);
	bool CalcAndAssign(const Variable& lhs, EToken calcOp, const Variable& rhs);
	bool CalcUnaryAndAssign(EToken unaryOp, const Variable& rhs);
};

}
