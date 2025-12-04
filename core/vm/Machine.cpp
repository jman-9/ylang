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
	_roff = 0;
	_sp = 0;
	_pc = 0;
	_retCode = INT_MAX;
	_spStack.push(0);
	_rpStack.push(0);
	_cspStack.push(0);
	//_regs.resize(1000);
	//_stack.resize(5000);

	_regs2.resize(1000);
	_stack2.resize(5000);

	//_literals.push_back({ ._type = Variable::_NULL_ });
	//_literals.push_back({ ._type = Variable::_TRUE_ });
	//_literals.push_back({ ._type = Variable::_FALSE_ });

	_literals2.resize(3);
	_literals2[0]._type = Variable2::_NULL_;
	_literals2[1]._type = Variable2::_TRUE_;
	_literals2[2]._type = Variable2::_FALSE_;

	ybuiltin::Garage::RegisterAll(_modMgr);
}

/*qaz
Variable* Machine::ResolveVar(ERefKind k, int idx)
{
	switch(k)
	{
	case ERefKind::Literal:
		{
			return &_literals[idx];
		}

	case ERefKind::Const: return &_consts[idx];

	case ERefKind::Reg:
		{
			_roff = idx;
			return &_regs[_rpStack.top() + idx];
		}
	case ERefKind::GlobalVar: return &_stack[idx];
	case ERefKind::LocalVar:
		{
			if(_sp < _cspStack.top() + idx + 1)
				_sp = _cspStack.top() + idx + 1;
			return &_stack[idx + _cspStack.top()];
		}

	case ERefKind::FieldVar:
		{
			if(_clsStack.empty()) throw 'n';

			return _clsStack.top()->_clso._fields[idx];
		}

	default: return nullptr;
	}
}*/
Variable2* Machine::ResolveVar2(ERefKind k, int idx)
{
	switch(k)
	{
	case ERefKind::Literal:
		{
			return &_literals2[idx];
		}

	case ERefKind::Const: return &_consts2[idx];

	case ERefKind::Reg:
		{
			_roff = idx;
			return &_regs2[_rpStack.top() + idx];
		}
	case ERefKind::GlobalVar: return &_stack2[idx];
	case ERefKind::LocalVar:
		{
			if(_sp < _cspStack.top() + idx + 1)
				_sp = _cspStack.top() + idx + 1;
			return &_stack2[idx + _cspStack.top()];
		}

	case ERefKind::FieldVar:
		{
			if(_clsStack2.empty()) throw 'n';

			return &_clsStack2.top()->_u._o->_clso._fields[idx];
		}

	default: return nullptr;
	}
}

void Machine::PushState()
{
	_rpStack.push(_rpStack.top() + _roff);
	_roffStack.push(_roff);
	_roff = 0;
	_retStack.push((uint32_t)_pc);
	_cspStack.push(_sp);
}

void Machine::PopState()
{
	_sp = _cspStack.top();
	_cspStack.pop();
	_rpStack.pop();
	_roff = _roffStack.top();
	_roffStack.pop();
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
		case EOpcode::LValueField: LValueField(*(Op::LValueField*)inst.code.data()); break;
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
		Variable2* src1 = ResolveVar2((ERefKind)as.src1Kind, as.src1);
		Variable2* src2 = ResolveVar2((ERefKind)as.src2Kind, as.src2);
		Variable2* dst = ResolveVar2((ERefKind)as.dstKind, as.dst);
		if(src1 && src2)
		{
			auto op = (EToken)as.op;
			if(Token::IsAssign(op))
			{
				src1->Assign(op, *src2);
				dst->SetVar(*src1);
			}
			else
			{
				dst->CalcAndAssign(*src1, op, *src2);
			}

			//TODO generalize
			if(*dst == Variable2::ATTR)
			{
				if(dst->attr().owner == Variable2::MODULE || dst->attr().owner == Variable2::MODULEOBJ)
				{
					auto found = dst->attr().owner.modObj()._mod.memberVars.find(dst->attr().name);
					if(found != dst->attr().owner.modObj()._mod.memberVars.end())
					{
						dst->SetValueFromContract(found->second);
					}
				}
				else if(dst->attr().owner == Variable2::CLASS)
				{
					auto found = dst->attr().owner.clsObj()._cls->_fieldMap.find(dst->_u._attr->name);
					if(found != dst->attr().owner.clsObj()._cls->_fieldMap.end())
					{
						dst->SetVar(dst->attr().owner.clsObj()._fields[found->second]);
					}
				}
			}
		}
		else if(src1)
		{
			if(*src1 == Variable2::LVREF)
			{//TODO
				auto lv = src1->_u._ref;
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
			if((ERefKind)as.src1Kind == ERefKind::Const || (ERefKind)as.src1Kind == ERefKind::Literal)
			{
				throw 'n';
			}

			Variable2* src2 = ResolveVar2((ERefKind)as.src2Kind, as.src2);
			Variable2* src1 = ResolveVar2((ERefKind)as.src1Kind, as.src1);
			src1->Assign((EToken)as.op, *src2);
		}
		else if(Token::IsIncDecOp((EToken)as.op))
		{
			Variable2* src2 = ResolveVar2((ERefKind)as.src2Kind, as.src2);
			Variable2* src1 = ResolveVar2((ERefKind)as.src1Kind, as.src1);
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

	if(cal.numPrms)
	{
		_roff -= (int)cal.numPrms - 1;
	}

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
	Variable2* test = ResolveVar2((ERefKind)jz.testKind, jz.test);

	if(	(*test == Variable2::INT && test->int_()) ||
		(*test == Variable2::FLOAT && test->float_()) ||
		(*test == Variable2::STR && !test->str().empty()) ||
		(*test == Variable2::CLASS && !test->clsObj()._cls->name.empty()) ||
		(*test == Variable2::MODULE && !test->mod().IsNull()) ||
		(*test == Variable2::MODULEOBJ && test->modObj()._mod.modDesc) ||
		//TODO(*test == Variable2::OBJECT && test->_obj) || || TODO qaz
		//(*test == Variable2::REF && test->ref()) || TODO qaz
		(*test == Variable2::ATTR && !test->attr().name.empty()) ||
		(*test == Variable2::LIST && test->_u._o) || // || TODO qaz
		(*test == Variable2::DICT && test->_u._o) || // || TODO qaz
		(*test == Variable2::_TRUE_))
	{
		return true;
	}

	_pc = jz.pos - 1;
	return true;
}

bool Machine::ListSet(const Op::ListSet& ls)
{
	Variable2* dst = ResolveVar2((ERefKind)ls.dstKind, ls.dst);

	Variable2* t = nullptr;
	if(*dst != Variable2::LVREF)
	{
		t = dst;
	}
	else
	{
		auto t = dst->_u._ref;
		dst->Clear();
	}
	t->SetList();
	//t->Clear();
	//TODO qazt->_list = new std::vector<Variable *>;
	//t->_type = Variable::LIST;
	return true;
}

bool Machine::ListAdd(const Op::ListAdd& la)
{
	Variable2* src = ResolveVar2((ERefKind)la.srcKind, la.src);
	Variable2* dst = ResolveVar2((ERefKind)la.dstKind, la.dst);
	if(*dst != Variable2::LIST)
	{
		throw 'n';
	}
	//TODOqazdst->_list->push_back(src->Clone());
	dst->list().push_back(*src);
	return true;
}

bool Machine::DictSet(const Op::DictSet& ds)
{
	Variable2* dst = ResolveVar2((ERefKind)ds.dstKind, ds.dst);

	Variable2* t = nullptr;
	if(*dst != Variable2::LVREF)
	{
		t = dst;
	}
	else
	{
		t = dst->_u._ref;
		dst->Clear();
	}
	t->SetDict();
	//qazt->Clear();
	//t->_dict = new std::unordered_map<std::string, Variable *>;
	//t->_type = Variable::DICT;
	return true;
}

bool Machine::DictAdd(const Op::DictAdd& da)
{
	Variable2* val = ResolveVar2((ERefKind)da.valKind, da.val);
	Variable2* key = ResolveVar2((ERefKind)da.keyKind, da.key);
	Variable2* dst = ResolveVar2((ERefKind)da.dstKind, da.dst);
	if(*dst != Variable2::DICT)
	{
		throw 'n';
	}
	if(*key != Variable2::STR)
	{
		throw 'n';
	}

	dst->dict()[*key->_u._s].SetVar(*val);
	//qaz(*dst->_dict)[key->_str] = val->Clone();
	return true;
}

bool Machine::Index(const Op::Index& li)
{
	Variable2* idx = ResolveVar2((ERefKind)li.idxKind, li.idx);
	Variable2* dst = ResolveVar2((ERefKind)li.dstKind, li.dst);

	if(*idx == Variable2::INT)
	{
		if(*dst != Variable2::LIST)
		{
			throw 'n';
		}

		//qaz*dst = *dst->_list->at(idx->_int);
		dst->SetVar(dst->list()[idx->int_()]);
	}
	else if(*idx == Variable2::STR)
	{
		if(*dst != Variable2::DICT)
		{
			throw 'n';
		}

		//qazauto found = dst->_dict->find(idx->_str);
		//if(found == dst->_dict->end())
		auto found = dst->dict().find(idx->str());
		if(found == dst->dict().end())
		{
			throw 'n';
		}

		dst->SetVar(found->second);
	}
	else
	{
		throw 'n';
	}
	return true;
}

bool Machine::LValueIndex(const Op::LValueIndex& lli)
{
	Variable2* idx = ResolveVar2((ERefKind)lli.idxKind, lli.idx);
	Variable2* dst = ResolveVar2((ERefKind)lli.dstKind, lli.dst);

	if(*idx == Variable2::INT)
	{
		Variable2* lst = nullptr;
		if(*dst == Variable2::LVREF)
		{
			if(dst->ref() != Variable2::LIST)
			{
				throw 'n';
			}
			lst = &dst->ref();
		}
		else if(*dst == Variable2::LIST)
		{
			lst = dst;
		}
		else
		{
			throw 'n';
		}

		auto& t = lst->list()[idx->int_()];
		dst->SetVarLVRef(t);
	}
	else if(*idx == Variable2::STR)
	{
		Variable2* dict = nullptr;
		if(*dst == Variable2::LVREF)
		{
			if(dst->ref() != Variable2::DICT)
			{
				throw 'n';
			}
			dict = &dst->ref();
		}
		else if(*dst == Variable2::DICT)
		{
			dict = dst;
		}
		else
		{
			throw 'n';
		}

		Variable2* t = nullptr;
		auto found = dict->dict().find(idx->str());
		if(found == dict->dict().end())
		{
			auto inserted = dict->dict().insert({idx->str(), Variable2()});
			if(!inserted.second)
			{
				throw 'n';
			}
			t = &inserted.first->second;
		}
		else
		{
			t = &found->second;
		}

		dst->SetVarLVRef(*t);
	}
	else
	{
		throw 'n';
	}
	return true;
}

bool Machine::Invoke(const Op::Invoke& ivk)
{
	if((ERefKind)ivk.dstKind == ERefKind::MemberFunc)
	{/*qaz
		if(_clsStack.empty())
		{//TODO
			throw 'n';
		}*/

		if(_clsStack2.empty())
		{//TODO
			throw 'n';
		}

		if(ivk.numArgs)
		{
			_roff -= (int)ivk.numArgs - 1;
		}

		//auto& cls = _clsStack.top()->_clso._cls;
		//Exec(cls._funcs[ivk.dst], 1);

		auto cls = _clsStack2.top()->clsObj()._cls;
		Exec(cls->_funcs[ivk.dst], 1);
		return true;
	}

	if((ERefKind)ivk.dstKind != ERefKind::Reg)
	{//TODO
		throw 'n';
	}

	Variable2* dst = ResolveVar2((ERefKind)ivk.dstKind, ivk.dst);
	if(*dst == Variable2::STR)
	{//TODO dynamic resolution
		//TBD
		throw 'n';
	}
	if(*dst != Variable2::ATTR)
	{
		throw 'n';
	}

	_roff = ivk.dst + 1;

	auto& owner = dst->attr().owner;
	if(owner == Variable2::CLASS)
	{
		auto cls = owner.clsObj()._cls;
		auto found = cls->_funcMap.find(dst->attr().name);
		if(found == cls->_funcMap.end())
		{//TODO
			throw 'n';
		}

		_clsStack2.push(&owner);
		Exec(cls->_funcs[found->second], 1);
		_clsStack2.pop();

		auto vs = ResolveVar2(ERefKind::Reg, ivk.dst + 1);
		auto vt = ResolveVar2(ERefKind::Reg, ivk.dst);
		vt->SetVar(*vs);
		return true;
	}

	const ymod::ModuleDesc* modDesc = primitive::GetModuleDesc(owner._type);
	if(!modDesc)
	{
		if(owner != Variable2::MODULE && owner != Variable2::MODULEOBJ)
		{//TODO
			throw 'n';
		}
		modDesc = owner.modObj()._mod.modDesc;
	}

	auto found = modDesc->memberTbl.find(dst->attr().name);
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
	if(found->second.needSelf || owner == Variable2::MODULEOBJ)
	{
		if(owner == Variable2::MODULE)
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
		{//TODO qaz
			//ya.args[0] = owner.ToContract();
		}
	}
	else
	{
		off = 0;
		ya.Reset(ivk.numArgs);
	}

	int argsRoff = ivk.dst + 1;

	if(modDesc->builtin)
	{
		for(int i=0; i<ivk.numArgs; i++)
		{//TODOqaz int value check
			auto arg = ResolveVar2(ERefKind::Reg, argsRoff+i);
			YArg yo { (void*)arg, YEArg::YVar };
			ya.args[i+off] = yo;
		}
	}
	else
	{
		for(int i=0; i<ivk.numArgs; i++)
		{//TODO int value check
			auto arg = ResolveVar2(ERefKind::Reg, argsRoff+i);
			//qazya.args[i+off] = arg->ToContract();
		}
	}

	auto ret = ResolveVar2(ERefKind::Reg, ivk.dst);
	ya.retBuff.tp = YEArg::YVar;
	ya.retBuff.o = ret;

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
			{//TODO qaz
				YObj* yo = (YObj*)yr.single.o;
				ret->SetModule( _modMgr.GetModuleDesc(yo->name.str), true);
				ret->_type = Variable2::MODULEOBJ;
				ret->modObj()._o = yo->obj;
				delete yo;
			}
			else if(yr.single.tp == YEArg::YVar)
			{//TODO no-op
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
	return true;
}

bool Machine::Inc(const Op::Inc& inc)
{
	Variable2* name = ResolveVar2(ERefKind::Const, inc.inc);

	if(!name || *name != Variable2::STR)
	{
		throw 'n';
	}

	const ymod::ModuleDesc& modDesc = _modMgr.GetModuleDesc(name->str());
	if(modDesc.IsNull())
	{//TODO
		throw 'n';
	}

	auto v = ResolveVar2(ERefKind::LocalVar, _sp);
	v->SetModule(modDesc, false);
	return true;
}

bool Machine::Jnz(const Op::Jnz& jnz)
{
	Variable2* test = ResolveVar2((ERefKind)jnz.testKind, jnz.test);

	if(	(*test == Variable2::INT && !test->int_()) ||
		(*test == Variable2::FLOAT && !test->float_()) ||
		(*test == Variable2::STR && test->str().empty()) ||
		//(*test == Variable2::OBJECT && !test->_obj) || //TODOqaz
		(*test == Variable2::CLASS && test->clsObj()._cls->name.empty()) ||
		(*test == Variable2::MODULE && test->mod().IsNull()) ||
		//(*test == Variable2::REF && !test->ref()) || TODOqaz
		(*test == Variable2::ATTR && test->attr().name.empty()) ||
		(*test == Variable2::LIST && !test->_u._o) ||	//TODOqaz
		(*test == Variable2::DICT && !test->_u._o) ||	//TODOqaz
		(*test == Variable2::MODULEOBJ && !test->modObj()._mod.modDesc) ||
		(*test == Variable2::_FALSE_) ||
		(*test == Variable2::_NULL_) ||
		(*test == Variable2::NONE))
	{
		return true;
	}

	_pc = jnz.pos - 1;
	return true;
}

bool Machine::NewMod(const Op::NewMod& nm)
{//TODO qaz
	/*
	Variable* dst = ResolveVar((ERefKind)nm.dstKind, nm.dst);
	if(*dst != Variable::STR)
	{//TODO
		throw 'n';
	}

	const auto& modDesc = _modMgr.GetModuleDesc(dst->_str);
	if(modDesc.IsNull())
	{//TODO
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

	auto v = ResolveVar(ERefKind::Reg, _roff);
	v->Clear();
	v->_type = Variable::OBJECT;
	v->_obj = yr.single.o;
	v->_mod = mod;
	return true;*/
	return false;
}

bool Machine::NewCls(const Op::NewCls& nc)
{
	Variable2* dst = ResolveVar2((ERefKind)nc.dstKind, nc.dst);
	if(*dst != Variable2::STR)
	{//TODO
		throw 'n';
	}

	auto found = _prg->_classTable.find(dst->str());
	if(found == _prg->_classTable.end())
	{//TODO
		throw 'n';
	}

	Variable2 v;
	//TODO qaz
	v.SetClass(found->second, true);

	_clsStack2.push(&v);

	int roffbk = _roff;
	_roff++;
	Exec(v.clsObj()._cls->_initer, 0);
	_roff = roffbk;

	if(nc.numArgs)
	{
		_roff -= (int)nc.numArgs - 1;
	}

	if(!v.clsObj()._cls->_ctor.empty())
	{
		Exec(v.clsObj()._cls->_ctor, 1);
	}

	//_clsStack.pop();
	_clsStack2.pop();

	dst = ResolveVar2(ERefKind::Reg, _roff);
	dst->SetVar(v);
	return true;
}

bool Machine::LValueField(const Op::LValueField& lvf)
{
	Variable2* fld = ResolveVar2((ERefKind)lvf.fieldKind, lvf.field);
	Variable2* dst = ResolveVar2((ERefKind)lvf.dstKind, lvf.dst);

	if(*fld != Variable2::STR)
	{//TODO
		throw 'n';
	}
	if(*dst != Variable2::CLASS)
	{//TODO
		throw 'n';
	}

	auto found = dst->clsObj()._cls->_fieldMap.find(fld->str());
	if(found == dst->clsObj()._cls->_fieldMap.end())
	{
		throw 'n';
	}
	dst->SetVarLVRef(dst->clsObj()._fields[found->second]);
	return true;
}


bool Machine::CallBuiltinFunc(const Op::Call& cal)
{
	if(cal.numPrms)
	{
		_roff -= (int)cal.numPrms - 1;
	}

	switch(cal.pos)
	{
	case 0xFFFF0000:
		if(cal.numPrms == 0)
		{//noop
		}
		else
		{
			auto v = ResolveVar2(ERefKind::Reg, _roff);
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
			auto v = ResolveVar2(ERefKind::Reg, _roff);
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
			auto v = ResolveVar2(ERefKind::Reg, _roff);
			_retCode = (int)v->int_();
		}
		break;

	default: throw 'n';
	}

	return true;
}


int Machine::Run(const Program& program, int start /* = 0 */)
{
	_prg = &program;
	_retCode = INT_MAX;

	/*qaz_consts.clear();

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
	}*/
	_consts2.clear();
	for(auto& c : _prg->_consts)
	{
		switch(c._type)
		{
		case Constant::INT:		_consts2.push_back({}); _consts2.back().SetInt(c._int); break;
		case Constant::FLOAT:	_consts2.push_back({}); _consts2.back().SetFloat(c._float); break;
		case Constant::STR:		_consts2.push_back({}); _consts2.back().SetStr(c._str); break;
		default: //TODO
			throw 'n';
		}
	}

	Exec(_prg->_mainCode, start);
	if(_retCode == INT_MAX) _retCode = 0;

	return _retCode;
}

}
