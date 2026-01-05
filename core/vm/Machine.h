#pragma once
#include "contract/Program.h"
#include "module/ModuleManager.h"
#include "Variable.h"
#include "VariableList.h"
#include <vector>
#include <stack>


namespace yvm
{

struct Caller
{
	std::string prgName;
	std::string prgPath;
	int line;
};

class Machine
{
public:
	Machine();

	int64_t Run(const Program& program, int start = 0);
	int64_t Continue(int start = -1);

protected:
	Variable* ResolveVar(ERefKind k, int idx);

	inline int64_t GoSub(Variable* callee, const Bytecode* sub, int start = 0);

	inline bool Exec(const Bytecode& code, int start = 0);

	const Program* _prg;
	Variable _prgObj;

	std::vector<Variable> _literals;
	VariableList _regs;
	VariableList _stack;
	std::stack<uint16_t> _spStack;
	std::stack<uint16_t> _rpStack;
	std::stack<uint16_t> _roffStack;
	std::stack<uint32_t> _retStack;
	std::stack<Variable*> _clsStack;
	std::stack<Variable*> _prgStack;
	std::stack<const Bytecode*> _codeStack;
	std::stack<Caller> _callStack;
	int _sp;
	int _roff;
	int _pc;
	int64_t _retCode;

	std::unordered_map<std::string, Variable> _prgObjTable;

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

	inline bool CreateClassObj(const Class& cls, int numArgs);
};

}
