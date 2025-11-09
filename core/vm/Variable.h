#pragma once
#include "../Token.h"
#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>



namespace yvm
{

struct Attribute;

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
		ATTR,
	};

	Type type = NONE;

	int64_t num = 0;
	std::string str = "";
	double _float = 0.0;
	void* obj = nullptr;
	std::vector<Variable*>* list;
	std::unordered_map<std::string, Variable*>* dict;
	Variable* ref = nullptr;
	Attribute* attr = nullptr;

	void Clear();
	void SetNum(int64_t num);
	void SetStr(const std::string& str);
	void SetList(const std::vector<Variable*>& list = std::vector<Variable*>());
	Variable* Clone();

	bool Assign(EToken op, const Variable& rval);
	bool CalcAndAssign(const Variable& lhs, EToken calcOp, const Variable& rhs);
	bool CalcUnaryAndAssign(EToken unaryOp, const Variable& rhs);

	std::string ToStr() const;

	bool operator==(Type cmp) const;
	bool operator!=(Type cmp) const;

	static Variable* NewNum(int64_t num = 0);
	static Variable* NewStr(const std::string& str = "");
	static Variable* NewList(const std::vector<Variable*>& list = std::vector<Variable*>());
};

struct Attribute
{
	Variable owner;
	std::string name;
};

}
