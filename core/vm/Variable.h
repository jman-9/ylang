#pragma once
#include "../Token.h"
#include "module/Module.h"
#include "../contract/ycontract.h"
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

		INT,
		FLOAT,
		STR,
		OBJECT,
		LIST,
		DICT,
		REF,
		ATTR,
		MODULE,
	};

	Type _type = NONE;

	int64_t _int = 0;
	double _float = 0.0;
	std::string _str = "";
	void* _obj = nullptr;
	std::vector<Variable*>* _list;
	std::unordered_map<std::string, Variable*>* _dict;
	Variable* _ref = nullptr;
	Attribute* _attr = nullptr;
	ymod::Module _mod;

	void Clear();
	void SetInt(int64_t num);
	void SetFloat(double f);
	void SetStr(const std::string& str);
	void SetList(const std::vector<Variable*>& list = std::vector<Variable*>());
	void SetModule(const ymod::Module& mod);
	void SetValueFromContract(YObj o);
	Variable* Clone();

	bool Assign(EToken op, const Variable& rval);
	bool CalcAndAssign(const Variable& lhs, EToken calcOp, const Variable& rhs);
	bool CalcUnaryAndAssign(EToken unaryOp, const Variable& rhs);

	YObj ToContract() const;
	std::string ToStr() const;

	bool operator==(Type cmp) const;
	bool operator!=(Type cmp) const;

	static Variable* NewNum(int64_t num = 0);
	static Variable* NewStr(const std::string& str = "");
	static Variable* NewList(const std::vector<Variable*>& list = std::vector<Variable*>());
	static Variable* New(YObj o);
};

struct Attribute
{
	Variable owner;
	std::string name;
};

}
