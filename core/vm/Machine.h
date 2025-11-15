#pragma once
#include "../Bytecode.h"
#include "module/ModuleManager.h"
#include "Variable.h"
#include <vector>
#include <stack>


namespace yvm
{

class Machine
{
public:
	Machine();

	int Run(const Bytecode& code, int start = 0);

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
	int _pc;
	int _retCode;

	ymod::ModuleManager _modMgr;

	inline bool Assign(const Op::Assign& as);
	inline bool PushSp();
	inline bool PopSp();
	inline bool Jmp(const Op::Jmp& jmp);
	inline bool Call(const Op::Call& cal);
	inline bool Ret();
	inline bool Jz(const Op::Jz& jz);
	inline bool ListSet(const Op::ListSet& ls);
	inline bool ListAdd(const Op::ListAdd& la);
	inline bool DictSet(const Op::DictSet& ds);
	inline bool DictAdd(const Op::DictAdd& da);
	inline bool Index(const Op::Index& li);
	inline bool LValueIndex(const Op::LValueIndex& lli);
	inline bool Invoke(const Op::Invoke& ivk);
	inline bool Inc(const Op::Inc& inc);
};

}
