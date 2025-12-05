#pragma once
#include "../Program.h"
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

	int Run(const Program& program, int start = 0);

protected:
	Variable* ResolveVar(ERefKind k, int idx);

	inline void PushState();
	inline void PopState();
	inline int Exec(const Bytecode& code, int start = 0);

	const Program* _prg;

	std::vector<Variable> _literals;
	std::vector<Variable> _consts;
	std::vector<Variable> _regs;
	std::vector<Variable> _stack;
	std::stack<uint16_t> _spStack;
	std::stack<uint16_t> _cspStack;
	std::stack<uint16_t> _rpStack;
	std::stack<uint16_t> _roffStack;
	std::stack<uint32_t> _retStack;
	std::stack<Variable*> _clsStack;
	int _sp;
	int _roff;
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
	inline bool Jnz(const Op::Jnz& jnz);
	inline bool NewMod(const Op::NewMod& nm);
	inline bool NewCls(const Op::NewCls& nc);
	inline bool LValueField(const Op::LValueField& lvf);

	inline bool CallBuiltinFunc(const Op::Call& cal);

	//TODO
	//inline bool InvokeClsFunc(const Variable& owner, const std::string& attrName);
	//inline bool InvokeModFunc(const Variable& owner, const std::string& attrName);
};

}
