#include "Machine.h"
#include "primitives/Primitives.h"
#include "builtin/BuiltinGarage.h"
#include <limits.h>
#include <format>
#include <iostream>
using namespace std;


namespace yvm
{

Machine::Machine()
{
	_prg = nullptr;
	_rp = 0;
	_sp = 0;
	_pc = 0;
	_retCode = INT_MAX;
	_spStack.push(0);
	_rpStack.push(0);
	_cspStack.push(0);
	_regs.resize(1000);
	_stack.resize(5000);

	ybuiltin::Garage::RegisterAll(_modMgr);
}


Variable* Machine::ResolveVar(ERefKind k, int idx)
{
	switch(k)
	{
	case ERefKind::Const: return &_consts[idx];

	case ERefKind::Reg:
		{
			_rp = _rpStack.top() + idx + 1;
			return &_regs[_rpStack.top() + idx];
		}
	case ERefKind::GlobalVar: return &_stack[idx];
	case ERefKind::LocalVar:
		{
			if(_sp < _cspStack.top() + idx + 1)
				_sp = _cspStack.top() + idx + 1;
			return &_stack[idx + _cspStack.top()];
		}

	case ERefKind::MemberVar:
		{
			if(_structStack.empty()) throw 'n';

			return _structStack.top()->_sto._members[idx];
		}

	default: return nullptr;
	}
}


void Machine::PushState()
{
	_rpStack.push(_rp);
	_retStack.push((uint32_t)_pc);
	_cspStack.push(_sp);
}

void Machine::PopState()
{
	_sp = _cspStack.top();
	_cspStack.pop();
	_rp = _rpStack.top() + 1;
	_rpStack.pop();
	_pc = _retStack.top();
	_retStack.pop();
}

int Machine::Exec(const Bytecode& code, int start /*= 0*/)
{
	PushState();

	for(_pc = start; _pc < code._code.size() && _retCode == INT_MAX; _pc++)
	{
		auto& inst = code._code[_pc];
		switch((EOpcode)inst.kind)
		{
		case EOpcode::Noop: break;
		case EOpcode::Assign: Assign(*(Op::Assign*)inst.code.data()); break;
		case EOpcode::PushSp: PushSp(); break;
		case EOpcode::PopSp: PopSp(); break;
		case EOpcode::Jmp:	Jmp(*(Op::Jmp*)inst.code.data()); break;
		case EOpcode::Call: Call(*(Op::Call*)inst.code.data()); break;
		case EOpcode::Ret: Ret(); return 0;
		case EOpcode::Jz: Jz(*(Op::Jz*)inst.code.data()); break;
		case EOpcode::ListSet: ListSet(*(Op::ListSet*)inst.code.data()); break;
		case EOpcode::ListAdd: ListAdd(*(Op::ListAdd*)inst.code.data()); break;
		case EOpcode::DictSet: DictSet(*(Op::DictSet*)inst.code.data()); break;
		case EOpcode::DictAdd: DictAdd(*(Op::DictAdd*)inst.code.data()); break;
		case EOpcode::Index: Index(*(Op::Index*)inst.code.data()); break;
		case EOpcode::LValueIndex: LValueIndex(*(Op::LValueIndex*)inst.code.data()); break;
		case EOpcode::Invoke: Invoke(*(Op::Invoke*)inst.code.data()); break;
		case EOpcode::Inc: Inc(*(Op::Inc*)inst.code.data()); break;
		case EOpcode::Jnz: Jnz(*(Op::Jnz*)inst.code.data()); break;
		case EOpcode::NewMod: NewMod(*(Op::NewMod*)inst.code.data()); break;
		case EOpcode::NewCls: NewCls(*(Op::NewCls*)inst.code.data()); break;
		default:
			throw 'n';//TODO
		}
	}

	PopState();
	return 0;
}

bool Machine::Assign(const Op::Assign& as)
{
	if((ERefKind)as.dstKind == ERefKind::Const)
	{
		throw 'n';
	}

	if((ERefKind)as.dstKind != ERefKind::None)
	{
		Variable* src1 = ResolveVar((ERefKind)as.src1Kind, as.src1);
		Variable* src2 = ResolveVar((ERefKind)as.src2Kind, as.src2);
		Variable* dst = ResolveVar((ERefKind)as.dstKind, as.dst);
		if(src1 && src2)
		{
			auto op = (EToken)as.op;
			if(Token::IsAssign(op))
			{
				src1->Assign(op, *src2);
				*dst = *src1;
			}
			else
			{
				dst->CalcAndAssign(*src1, (EToken)as.op, *src2);
			}

			//TODO generalize
			if(*dst == Variable::ATTR)
			{
				if(dst->_attr->owner == Variable::MODULE)
				{
					auto found = dst->_attr->owner._mod.memberVars.find(dst->_attr->name);
					if(found != dst->_attr->owner._mod.memberVars.end())
					{
						dst->SetValueFromContract(found->second);
					}
				}
			}
		}
		else if(src1)
		{
			if(*src1 == Variable::LVREF)
			{
				auto lv = src1->_ref;
				dst->Assign(EToken::Assign, *lv);
				src1 = lv;
			}
			else
			{
				dst->Assign(EToken::Assign, *src1);
			}
			src1->CalcIncDec((EToken)as.op);

		}
		else if(src2)
		{
			if(Token::IsIncDecOp((EToken)as.op))
			{
				src2->CalcIncDec((EToken)as.op);
				dst->Assign(EToken::Assign, *src2);
			}
			else
			{
				dst->CalcUnaryAndAssign((EToken)as.op, *src2);
			}
		}
		else
		{
			throw 'n';
		}
	}
	else
	{
		if(Token::IsAssign((EToken)as.op))
		{
			if((ERefKind)as.src1Kind == ERefKind::Const)
			{
				throw 'n';
			}

			Variable* src2 = ResolveVar((ERefKind)as.src2Kind, as.src2);
			Variable* src1 = ResolveVar((ERefKind)as.src1Kind, as.src1);
			src1->Assign((EToken)as.op, *src2);
		}
		else if(Token::IsIncDecOp((EToken)as.op))
		{
			Variable* src2 = ResolveVar((ERefKind)as.src2Kind, as.src2);
			Variable* src1 = ResolveVar((ERefKind)as.src1Kind, as.src1);
			if(src1)
			{
				src1->CalcIncDec((EToken)as.op);
			}
			else if(src2)
			{
				src2->CalcIncDec((EToken)as.op);
			}
			else
			{
				throw 'n';
			}

		}
		else if(Token::IsPrefixUnary((EToken)as.op))
		{//no-op
		}
		else
		{//TODO func call
			throw 'n';
		}
	}

	return true;
}

bool Machine::PushSp()
{
	_spStack.push(_sp);
	return true;
}

bool Machine::PopSp()
{
	_sp = _spStack.top();
	_spStack.pop();
	return true;
}

bool Machine::Jmp(const Op::Jmp& jmp)
{
	_pc = jmp.pos - 1;
	return true;
}

bool Machine::Call(const Op::Call& cal)
{
	if(cal.pos >= 0xFFFF0000)
	{
		return CallBuiltinFunc(cal);
	}

	_rp -= (int)cal.numPrms;

	if(cal.seg == 0)
	{//TODO
		return Exec(_prg->_mainCode, cal.pos);
	}
	else
	{//TODO
		throw 'n';
	}
	return true;
}

bool Machine::Ret()
{
	PopState();
	return true;
}

bool Machine::Jz(const Op::Jz& jz)
{
	Variable* test = ResolveVar((ERefKind)jz.testKind, jz.test);
	if(!test->_int)
	{
		_pc = jz.pos - 1;
	}
	return true;
}

bool Machine::ListSet(const Op::ListSet& ls)
{
	Variable* dst = ResolveVar((ERefKind)ls.dstKind, ls.dst);

	Variable* t = nullptr;
	if(dst->_type != Variable::LVREF)
	{
		t = dst;
	}
	else
	{
		auto t = dst->_ref;
		dst->Clear();
	}
	t->Clear();
	t->_list = new std::vector<Variable *>;
	t->_type = Variable::LIST;
	return true;
}

bool Machine::ListAdd(const Op::ListAdd& la)
{
	Variable* src = ResolveVar((ERefKind)la.srcKind, la.src);
	Variable* dst = ResolveVar((ERefKind)la.dstKind, la.dst);
	if(dst->_type != Variable::LIST)
	{
		throw 'n';
	}
	dst->_list->push_back(src->Clone());
	return true;
}

bool Machine::DictSet(const Op::DictSet& ds)
{
	Variable* dst = ResolveVar((ERefKind)ds.dstKind, ds.dst);

	Variable* t = nullptr;
	if(dst->_type != Variable::LVREF)
	{
		t = dst;
	}
	else
	{
		t = dst->_ref;
		dst->Clear();
	}
	t->Clear();
	t->_dict = new std::unordered_map<std::string, Variable *>;
	t->_type = Variable::DICT;
	return true;
}

bool Machine::DictAdd(const Op::DictAdd& da)
{
	Variable* val = ResolveVar((ERefKind)da.valKind, da.val);
	Variable* key = ResolveVar((ERefKind)da.keyKind, da.key);
	Variable* dst = ResolveVar((ERefKind)da.dstKind, da.dst);
	if(dst->_type != Variable::DICT)
	{
		throw 'n';
	}
	if(key->_type != Variable::STR)
	{
		throw 'n';
	}

	(*dst->_dict)[key->_str] = val->Clone();
	return true;
}

bool Machine::Index(const Op::Index& li)
{
	Variable* idx = ResolveVar((ERefKind)li.idxKind, li.idx);
	Variable* dst = ResolveVar((ERefKind)li.dstKind, li.dst);

	if(idx->_type == Variable::INT)
	{
		if(dst->_type != Variable::LIST)
		{
			throw 'n';
		}

		*dst = *dst->_list->at(idx->_int);
	}
	else if(idx->_type == Variable::STR)
	{
		if(dst->_type != Variable::DICT)
		{
			throw 'n';
		}

		auto found = dst->_dict->find(idx->_str);
		if(found == dst->_dict->end())
		{
			throw 'n';
		}

		*dst = *found->second;
	}
	else
	{
		throw 'n';
	}
	return true;
}

bool Machine::LValueIndex(const Op::LValueIndex& lli)
{
	Variable* idx = ResolveVar((ERefKind)lli.idxKind, lli.idx);
	Variable* dst = ResolveVar((ERefKind)lli.dstKind, lli.dst);

	if(idx->_type == Variable::INT)
	{
		if(dst->_type != Variable::LIST)
		{
			throw 'n';
		}

		auto t = dst->_list->at(idx->_int);
		dst->Clear();
		dst->_type = Variable::LVREF;
		dst->_ref = t;
	}
	else if(idx->_type == Variable::STR)
	{
		if(dst->_type != Variable::DICT)
		{
			throw 'n';
		}

		Variable* t = nullptr;
		auto found = dst->_dict->find(idx->_str);
		if(found == dst->_dict->end())
		{
			auto inserted = dst->_dict->insert({idx->_str, new Variable()});
			if(!inserted.second)
			{
				throw 'n';
			}
			t = inserted.first->second;
		}
		else
		{
			t = found->second;
		}

		dst->Clear();
		dst->_type = Variable::LVREF;
		dst->_ref = t;
	}
	else
	{
		throw 'n';
	}
	return true;
}

bool Machine::Invoke(const Op::Invoke& ivk)
{
	int rpBackup = _rp - ivk.numArgs - 1;
	int argsRp = rpBackup + 1;

	Variable* dst = ResolveVar((ERefKind)ivk.dstKind, ivk.dst);
	if(*dst == Variable::STR)
	{//TODO dynamic resolution
		//TBD
		throw 'n';
	}
	if(dst->_type != Variable::ATTR)
	{
		throw 'n';
	}

	auto& owner = dst->_attr->owner;
	if(owner == Variable::STRUCT)
	{
		auto found = owner._sto._st._funcTable.find(dst->_attr->name);
		if(found == owner._sto._st._funcTable.end())
		{//TODO
			throw 'n';
		}

		_structStack.push(&owner);
		Exec(found->second, 1);
		_structStack.pop();
		//TODO
		auto vs = ResolveVar(ERefKind::Reg, _rp-1);
		_rp = rpBackup;
		auto vt = ResolveVar(ERefKind::Reg, _rp);
		*vt = *vs;
		return true;
	}

	const ymod::ModuleDesc* modDesc = primitive::GetModuleDesc(owner._type);
	if(!modDesc)
	{
		if(owner != Variable::MODULE && owner != Variable::OBJECT)
		{//TODO
			throw 'n';
		}
		modDesc = owner._mod.modDesc;
	}

	auto found = modDesc->memberTbl.find(dst->_attr->name);
	if(found == modDesc->memberTbl.end())
	{//TODO
		throw 'n';
	}

	if(ivk.numArgs < found->second.numPrms)
	{//TODO
		throw 'n';
	}

	YArgs ya;
	int off = 0;
	if(found->second.needSelf)
	{
		if(owner == Variable::MODULE)
		{//TODO
			throw 'n';
		}

		off = 1;
		ya.Reset(ivk.numArgs + 1);
		if(modDesc->builtin)
		{
			ya.args[0].tp = YEArg::YVar;
			ya.args[0].o = &owner;
		}
		else
		{//TODO
			ya.args[0] = owner.ToContract();
		}
	}
	else
	{
		off = 0;
		ya.Reset(ivk.numArgs);
	}

	if(modDesc->builtin)
	{
		for(int i=0; i<ivk.numArgs; i++)
		{//TODO int value check
			auto arg = ResolveVar(ERefKind::Reg, argsRp+i);
			YArg yo { (void*)arg, YEArg::YVar };
			ya.args[i+off] = yo;
		}
	}
	else
	{
		for(int i=0; i<ivk.numArgs; i++)
		{//TODO int value check
			auto arg = ResolveVar(ERefKind::Reg, argsRp+i);
			ya.args[i+off] = arg->ToContract();
		}
	}

	Variable* ret = nullptr;
	auto yr = found->second.func(&ya);
	if(yr.code)
	{//TODO
		throw 'n';
	}
	if(modDesc->builtin)
	{
		if(yr.single.tp != YEArg::None)
		{
			if(yr.single.tp == YEArg::Object)
			{//TODO
				YObj* yo = (YObj*)yr.single.o;
				ymod::Module mod { &_modMgr.GetModuleDesc(yo->name.str) };
				ret = Variable::NewObject(mod, yo->obj);
			}
			else if(yr.single.tp == YEArg::YVar)
			{//TODO
				ret = (Variable*)yr.single.o;
			}
			else
			{
				throw 'n';
			}
		}
		else if(yr.vals.sz != 0)
		{//TODO
		}
		else
		{//TODO no return
		}
	}
	else
	{//TODO
	}

	_rp = rpBackup;
	auto v = ResolveVar(ERefKind::Reg, _rp);
	*v = ret ? *ret : Variable();
	return true;
}

bool Machine::Inc(const Op::Inc& inc)
{
	Variable* name = ResolveVar(ERefKind::Const, inc.inc);

	if(!name || *name != Variable::STR)
	{
		throw 'n';
	}

	const ymod::ModuleDesc& modDesc = _modMgr.GetModuleDesc(name->_str);
	if(modDesc.IsNull())
	{//TODO
		throw 'n';
	}

	ymod::Module mod{ &modDesc, };
	if(modDesc.initer)
	{
		mod = modDesc.initer();
	}

	auto v = ResolveVar(ERefKind::LocalVar, _sp);
	v->SetModule(mod);
	return true;
}

bool Machine::Jnz(const Op::Jnz& jnz)
{
	Variable* test = ResolveVar((ERefKind)jnz.testKind, jnz.test);
	if(test->_int)
	{
		_pc = jnz.pos - 1;
	}
	return true;
}

bool Machine::NewMod(const Op::NewMod& nm)
{
	int rpBackup = _rp - nm.numArgs - 1;
	int argsRp = rpBackup + 1;

	Variable* dst = ResolveVar((ERefKind)nm.dstKind, nm.dst);
	if(*dst != Variable::STR)
	{//TOPDO
		throw 'n';
	}

	const auto& modDesc = _modMgr.GetModuleDesc(dst->_str);
	if(modDesc.IsNull())
	{//TOPDO
		throw 'n';
	}

	if(!modDesc.newer)
	{//TODO
		throw 'n';
	}

	YRet yr = modDesc.newer(nullptr);
	if(yr.single.tp != YEArg::Object)
	{
		throw 'n';
	}

	ymod::Module mod { .modDesc = &modDesc };
	if(modDesc.initer)
	{
		mod = modDesc.initer();
	}

	//_rpStack.pop(); //TODO confirm
	_rp = rpBackup;
	auto v = ResolveVar(ERefKind::Reg, 0);
	v->Clear();
	v->_type = Variable::OBJECT;
	v->_obj = yr.single.o;
	v->_mod = mod;
	return true;
}

bool Machine::NewCls(const Op::NewCls& nc)
{
	int rpBackup = _rp - nc.numArgs - 1;
	int argsRp = rpBackup + 1;

	Variable* dst = ResolveVar((ERefKind)nc.dstKind, nc.dst);
	if(*dst != Variable::STR)
	{//TODO
		throw 'n';
	}

	auto found = _prg->_structTable.find(dst->_str);
	if(found == _prg->_structTable.end())
	{//TODO
		throw 'n';
	}

	auto v = ResolveVar(ERefKind::Reg, 0);
	v->Clear();
	v->_type = Variable::STRUCT;
	v->_sto._st = found->second;
	for(size_t i=0; i<v->_sto._st._fields.size(); i++)
	{
		v->_sto._members.push_back(new Variable());
	}
	_structStack.push(v);
	_retStack.push(_pc);
	//TODO
	_pc = 0;
	Exec(v->_sto._st._initer);
	_pc = _retStack.top();
	_retStack.pop();
	_structStack.pop();
	return true;
}

bool Machine::CallBuiltinFunc(const Op::Call& cal)
{
	_rp -= (int)cal.numPrms;
	_rpStack.push(_rp);

	switch(cal.pos)
	{
	case 0xFFFF0000:
		if(cal.numPrms == 0)
		{//noop
		}
		else
		{
			auto v = ResolveVar(ERefKind::Reg, 0);
			cout << v->ToStr();
		}
		break;

	case 0xFFFF0000 + 1:
		if(cal.numPrms == 0)
		{
			cout << "\n";
		}
		else
		{
			auto v = ResolveVar(ERefKind::Reg, 0);
			cout << v->ToStr() << "\n";
		}
		break;

	case 0xFFFF0000 + 2:
		if(cal.numPrms == 0)
		{
			_retCode = 0;
		}
		else
		{
			auto v = ResolveVar(ERefKind::Reg, 0);
			_retCode = v->_int;
		}
		break;

	default: throw 'n';
	}

	_rp = _rpStack.top();
	_rpStack.pop();
	return true;
}


int Machine::Run(const Program& program, int start /* = 0 */)
{
	_prg = &program;
	_retCode = INT_MAX;

	_consts.clear();
	for(auto& c : _prg->_consts)
	{
		switch(c._type)
		{
		case Constant::INT:		_consts.push_back({ ._type = Variable::INT, ._int = c._int }); break;
		case Constant::FLOAT:	_consts.push_back({ ._type = Variable::FLOAT, ._float = c._float }); break;
		case Constant::STR:		_consts.push_back({ ._type = Variable::STR, ._str = c._str }); break;
		default: //TODO
			throw 'n';
		}
	}

	Exec(_prg->_mainCode, start);
	if(_retCode == INT_MAX) _retCode = 0;

	return _retCode;
}

}
