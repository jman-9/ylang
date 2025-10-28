#pragma once
#include "../Token.h"
#include "../Bytecode.h"
#include <stack>

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
	};

	Type type = NONE;

	int64_t num = 0;
	std::string str = "";
	double _float = 0.0;
	void* obj = nullptr;

	void Clear();

	bool Assign(EToken op, const Variable& rval);
	bool CalcAndAssign(const Variable& lhs, EToken calcOp, const Variable& rhs);
};

class Machine
{
public:
	Machine();

	void Run(const Bytecode& code);

	std::vector<Variable> _consts;
	std::vector<Variable> _regs;
	std::vector<Variable> _stack;
	std::stack<uint16_t> _spStack;

protected:
	Variable* ResolveVar(RefKind k, int idx);
};

}
