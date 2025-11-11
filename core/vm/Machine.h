#pragma once
#include "../Bytecode.h"
#include "Variable.h"
#include "ModuleManager.h"
#include <vector>
#include <stack>

namespace yvm
{

class Machine
{
public:
	Machine();

	void Run(const Bytecode& code, int start = 0);

protected:
	Variable* ResolveVar(ERefKind k, int idx);

	std::vector<Variable> _consts;
	std::vector<Variable> _regs;
	std::vector<Variable> _stack;
	std::stack<uint16_t> _spStack;
	std::stack<uint16_t> _cspStack;
	std::stack<uint16_t> _rpStack;
	std::stack<uint32_t> _retStack;
	int _sp;
	int _rp;

	ModuleManager _modMgr;
};

}
