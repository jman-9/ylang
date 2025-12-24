#include "Machine.h"
#include "primitives/Primitives.h"
#include "builtin/BuiltinGarage.h"
#include "RuntimeError.h"
#include <limits.h>
#include <format>
#include <string>
#include <iostream>
using namespace std;


namespace yvm
{

struct YArgsAuto
{
	YArgsAuto() {}
	~YArgsAuto() { if(_ya.args) _ya.Reset(0); }
	YArgs _ya;
};


Machine::Machine()
{
	_prg = nullptr;
	_roff = 0;
	_sp = 0;
	_pc = 0;
	_retCode = INT_MAX;
	_rpStack.push(0);

	//TODO remove hardcoding
	_literals.resize(3);
	_literals[0]._type = Variable::_NULL_;
	_literals[1]._type = Variable::_TRUE_;
	_literals[2]._type = Variable::_FALSE_;

	ybuiltin::Garage::RegisterAll(_modMgr);
}

Variable* Machine::ResolveVar(ERefKind k, int idx)
{
	switch(k)
	{
	case ERefKind::Literal:
		{
			return &_literals[idx];
		}

	case ERefKind::Const:
		{
			auto& prgObj = _prgStack.top()->prgObj();
			auto& consts = prgObj._consts;

			auto cv = consts.Get(idx);

			if(*cv == Variable::NONE)
			{
				auto c = prgObj._prg->_consts[idx];
				switch(c._type)
				{
				case Constant::INT:		cv->SetInt(c._int); break;
				case Constant::FLOAT:	cv->SetFloat(c._float); break;
				case Constant::STR:		cv->SetStr(c._str); break;
				default: //TODO
					INTERNALERR(format("{}: unsupported constant type", (int)c._type));
				}
			}
			return cv;
		}

	case ERefKind::Reg:
		{
			_roff = idx;
			return _regs.Get(_rpStack.top() + idx);
		}
	case ERefKind::GlobalVar:
		{
			auto& glb = _prgStack.top()->prgObj()._globals;
			return glb.Get(idx);
		}
	case ERefKind::LocalVar:
		{
			if(_sp < _spStack.top() + idx + 1)
				_sp = _spStack.top() + idx + 1;

			return _stack.Get(_spStack.top() + idx);
		}

	case ERefKind::FieldVar:
		{
			if(_clsStack.empty())
				INTERNALERR("no class object for field variable");

			return &_clsStack.top()->clsObj()._fields[idx];
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
	_spStack.push(_sp);
}

void Machine::PopState()
{
	_sp = _spStack.top();
	_spStack.pop();
	_rpStack.pop();
	_roff = _roffStack.top();
	_roffStack.pop();
	_pc = _retStack.top();
	_retStack.pop();
}

bool Machine::ExecInst(const Instruction& inst)
{
	switch((EOpcode)inst.kind)
	{
	case EOpcode::Noop: break;
	case EOpcode::Assign: Assign(*(Op::Assign*)inst.code.data()); break;
	case EOpcode::PushSp: PushSp(); break;
	case EOpcode::PopSp: PopSp(); break;
	case EOpcode::Jmp:	Jmp(*(Op::Jmp*)inst.code.data()); break;
	case EOpcode::Call: Call(*(Op::Call*)inst.code.data()); break;
	case EOpcode::Ret: Ret(); return true;
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
		INTERNALERR(format("'{}': unknown opcode", inst.kind));//TODO
	}
	return false;
}

int Machine::Exec(const Bytecode& code, int start /* = 0 */)
{
	PushState();

	for(_pc = start; _pc < code._code.size() && _retCode == INT_MAX; _pc++)
	{
		int pc = _pc;
		try
		{
			if(ExecInst(code._code[_pc])) break;
		}
		catch(RuntimeError e)
		{
			if(e._srcLine < 0)
			{
				e._srcLine = code._srcLines[pc];
				e._bytecodeLine = pc;
			}
			throw e;
		}
	}

	PopState();
	return 0;
}

bool Machine::Assign(const Op::Assign& as)
{
	if((ERefKind)as.dstKind == ERefKind::Const)
	{
		INTERNALERR(format("cannot assign to const"));
	}

	auto op = (EToken)as.op;

	if((ERefKind)as.dstKind != ERefKind::None)
	{
		Variable* src1 = ResolveVar((ERefKind)as.src1Kind, as.src1);
		Variable* src2 = ResolveVar((ERefKind)as.src2Kind, as.src2);
		Variable* dst = ResolveVar((ERefKind)as.dstKind, as.dst);
		if(src1 && src2)
		{

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
			if(*dst == Variable::ATTR)
			{
				if(dst->attr().owner == Variable::MODULE || dst->attr().owner == Variable::MODULEOBJ)
				{
					auto found = dst->attr().owner.modObj()._mod.memberVars.find(dst->attr().name);
					if(found != dst->attr().owner.modObj()._mod.memberVars.end())
					{
						dst->SetValueFromContract(found->second);
					}
					else
					{//TODO optimize
						auto& o = dst->attr().owner;
						if(!o.modObj()._mod.modDesc->memberTbl.contains(dst->attr().name))
						{
							throw RuntimeError::NoMember(o._type, o.modObj()._mod.modDesc->name, dst->attr().name);
						}
					}
				}
				else if(dst->attr().owner == Variable::CLASSOBJ)
				{
					auto found = dst->attr().owner.clsObj()._cls->_fieldMap.find(dst->attr().name);
					if(found != dst->attr().owner.clsObj()._cls->_fieldMap.end())
					{
						dst->SetVar(dst->attr().owner.clsObj()._fields[found->second]);
					}
					else
					{	//TODO optimize
						auto& o = dst->attr().owner;
						if(!o.clsObj()._cls->_funcMap.contains(dst->attr().name))
						{
							throw RuntimeError::NoMember(o._type, o.clsObj()._cls->name, dst->attr().name);
						}
					}
				}
				else if(dst->attr().owner == Variable::PROGRAMOBJ)
				{
					auto found = dst->attr().owner.prgObj()._prg->_globalTable.find(dst->attr().name);
					if(found != dst->attr().owner.prgObj()._prg->_globalTable.end())
					{
						if(found->second.kind == EGlobalSymbol::Var)
						{
							dst->SetVar(*dst->attr().owner.prgObj()._globals.Get(found->second.idx));
						}
					}
					else
					{//TODO qaz
					}
				}
				//TODO static class field
			}
		}
		else if(src1)
		{
			if(*src1 == Variable::LVREF)
			{
				auto t = *src1;
				dst->Assign(EToken::Assign, src1->lvref());
				t.CalcIncDec(op);
			}
			else
			{
				dst->Assign(EToken::Assign, *src1);
				src1->CalcIncDec(op);
			}
		}
		else if(src2)
		{
			if(Token::IsIncDecOp(op))
			{
				src2->CalcIncDec(op);
				dst->Assign(EToken::Assign, *src2);
			}
			else
			{
				dst->CalcUnaryAndAssign(op, *src2);
			}
		}
		else
		{//TODO
			INTERNALERR(format("no source variable for '{}'", Token::TokenString(op)));
		}
	}
	else
	{
		if(Token::IsAssign(op))
		{
			if((ERefKind)as.src1Kind == ERefKind::Const || (ERefKind)as.src1Kind == ERefKind::Literal)
			{
				INTERNALERR(format("cannot assign to const"));
			}

			Variable* src2 = ResolveVar((ERefKind)as.src2Kind, as.src2);
			Variable* src1 = ResolveVar((ERefKind)as.src1Kind, as.src1);
			src1->Assign(op, *src2);
		}
		else if(Token::IsIncDecOp(op))
		{
			Variable* src2 = ResolveVar((ERefKind)as.src2Kind, as.src2);
			Variable* src1 = ResolveVar((ERefKind)as.src1Kind, as.src1);
			if(src1)
			{
				src1->CalcIncDec(op);
			}
			else if(src2)
			{
				src2->CalcIncDec(op);
			}
			else
			{
				INTERNALERR(format("no operand for '{}'", Token::TokenString(op)));
			}

		}
		else if(Token::IsPrefixUnary(op))
		{//no-op
		}
		else
		{//TODO trace
			// currently no-op
		}
	}

	return true;
}

bool Machine::PushSp()
{//TODO usability?
	//_spStack.push(_sp);
	return true;
}

bool Machine::PopSp()
{//TODO usability?
	//_sp = _spStack.top();
	//_spStack.pop();
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

	//TODO check
	/*if(cal.numPrms)
	{
		_roff -= (int)cal.numPrms - 1;
	}*/
	//TODO check
	if((ERefKind)cal.dstKind != ERefKind::Reg)
	{//TODO
		INTERNALERR(format("'{}': A return value must be assigned to a register({})", cal.dstKind, (int)ERefKind::Reg));
	}
	_roff = cal.dst;

	if(cal.seg == 0)
	{//TODO
		Exec(_prgStack.top()->prgObj()._prg->_mainCode, cal.pos);
	}
	else
	{//TODO
		INTERNALERR(format("'{}': unsupported call segment", cal.seg));
	}
	return true;
}

bool Machine::Ret()
{
	//PopState();
	return true;
}

bool Machine::Jz(const Op::Jz& jz)
{
	Variable* test = ResolveVar((ERefKind)jz.testKind, jz.test);

	if(!test->IsNullOrFalse())
		return true;

	_pc = jz.pos - 1;
	return true;
}

bool Machine::ListSet(const Op::ListSet& ls)
{
	Variable* dst = ResolveVar((ERefKind)ls.dstKind, ls.dst);

	if(*dst != Variable::LVREF)
	{
		dst->SetList();
	}
	else
	{
		auto t = &dst->lvref();
		t->SetList();
		dst->SetVar(*t);
	}
	return true;
}

bool Machine::ListAdd(const Op::ListAdd& la)
{
	Variable* src = ResolveVar((ERefKind)la.srcKind, la.src);
	Variable* dst = ResolveVar((ERefKind)la.dstKind, la.dst);
	if(*dst != Variable::LIST)
	{
		INTERNALERR(format("'{}': unsupported for ListAdd", dst->TypeStr()));
	}
	dst->list().push_back(*src);
	return true;
}

bool Machine::DictSet(const Op::DictSet& ds)
{
	Variable* dst = ResolveVar((ERefKind)ds.dstKind, ds.dst);

	if(*dst != Variable::LVREF)
	{
		dst->SetDict();
	}
	else
	{
		auto t = &dst->lvref();
		t->SetDict();
		dst->SetVar(*t);
	}
	return true;
}

bool Machine::DictAdd(const Op::DictAdd& da)
{
	Variable* val = ResolveVar((ERefKind)da.valKind, da.val);
	Variable* key = ResolveVar((ERefKind)da.keyKind, da.key);
	Variable* dst = ResolveVar((ERefKind)da.dstKind, da.dst);
	if(*dst != Variable::DICT)
	{
		INTERNALERR(format("'{}': unsupported for DictAdd", dst->TypeStr()));
	}
	if(*key != Variable::STR)
	{
		INTERNALERR(format("'{}': unsupported key for DictAdd", key->TypeStr()));
	}

	dst->dict()[*key->_u._s].SetVar(*val);
	return true;
}

bool Machine::Index(const Op::Index& li)
{
	Variable* idx = ResolveVar((ERefKind)li.idxKind, li.idx);
	Variable* dst = ResolveVar((ERefKind)li.dstKind, li.dst);

	if(*idx == Variable::INT)
	{
		if(*dst == Variable::STR)
		{
			if(idx->int_() < 0 || idx->int_() >= dst->str().size())
			{//qaz TODO
				throw RuntimeError::OutOfRange(dst->_type, "str", idx->int_(), dst->str().size());
			}
			dst->SetStr(string() + dst->str()[idx->int_()]);
		}
		else if(*dst == Variable::LIST)
		{
			if(idx->int_() < 0 || idx->int_() >= dst->list().size())
			{//qaz TODO
				throw RuntimeError::OutOfRange(dst->_type, "list", idx->int_(), dst->list().size());
			}
			dst->SetVar(dst->list()[idx->int_()]);
		}
		else
		{
			throw RuntimeError::UnsupportedType(dst->_type, "", to_string(idx->int_()));
			//TODO qaz
		}


	}
	else if(*idx == Variable::STR)
	{
		if(*dst != Variable::DICT)
		{
			throw RuntimeError::UnsupportedType(dst->_type, "", idx->str());
			//TODO qaz
		}

		auto found = dst->dict().find(idx->str());
		if(found == dst->dict().end())
		{
			throw RuntimeError::NotFound(dst->_type, "dict", idx->str());
			//TODO qaz
		}

		dst->SetVar(found->second);
	}
	else
	{
		throw RuntimeError::UnsupportedType(dst->_type, "", idx->ToStr());
		//TODO qaz
	}
	return true;
}

bool Machine::LValueIndex(const Op::LValueIndex& lli)
{
	Variable* idx = ResolveVar((ERefKind)lli.idxKind, lli.idx);
	Variable* dst = ResolveVar((ERefKind)lli.dstKind, lli.dst);

	if(*idx == Variable::INT)
	{
		Variable* lst = nullptr;
		if(*dst == Variable::LVREF)
		{
			if(dst->lvref() != Variable::LIST)
			{//qaz todo
				throw RuntimeError::UnsupportedType(dst->lvref()._type, "", to_string(idx->int_()));
			}
			lst = &dst->lvref();
		}
		else if(*dst == Variable::LIST)
		{
			lst = dst;
		}
		else
		{
			throw RuntimeError::UnsupportedType(dst->_type, "", to_string(idx->int_()));
			//qaz TODO
		}

		auto& t = lst->list()[idx->int_()];
		dst->SetVarLVRef(t);
	}
	else if(*idx == Variable::STR)
	{
		Variable* dict = nullptr;
		if(*dst == Variable::LVREF)
		{
			if(dst->lvref() != Variable::DICT)
			{// qaz todo
				throw RuntimeError::UnsupportedType(dst->lvref()._type, "", idx->str());
			}
			dict = &dst->lvref();
		}
		else if(*dst == Variable::DICT)
		{
			dict = dst;
		}
		else
		{
			throw RuntimeError::UnsupportedType(dst->_type, "", idx->str());
			//TODO qaz
		}

		Variable* t = nullptr;
		auto found = dict->dict().find(idx->str());
		if(found == dict->dict().end())
		{
			auto inserted = dict->dict().insert({idx->str(), Variable()});
			if(!inserted.second)
			{
				INTERNALERR(format("'{}': internal error at dict().insert()", idx->str()));
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
		throw RuntimeError::UnsupportedType(dst->_type, "", to_string(idx->float_()));
		//qaz TODO
	}
	return true;
}

bool Machine::Invoke(const Op::Invoke& ivk)
{
	if((ERefKind)ivk.dstKind == ERefKind::MemberFunc)
	{
		if(_clsStack.empty())
		{//TODO
			INTERNALERR("no class object for member function");
		}

		if(ivk.numArgs)
		{
			_roff -= (int)ivk.numArgs - 1;
		}

		auto cls = _clsStack.top()->clsObj()._cls;
		Exec(cls->_funcs[ivk.dst], 1);
		return true;
	}

	if((ERefKind)ivk.dstKind != ERefKind::Reg)
	{//TODO
		INTERNALERR(format("'{}': A return value must be assigned to a register({})", ivk.dstKind, (int)ERefKind::Reg));
	}

	Variable* dst = ResolveVar((ERefKind)ivk.dstKind, ivk.dst);
	if(*dst == Variable::STR)
	{//TODO dynamic resolution
		//TBD
		INTERNALERR(format("'{}': not implemented", dst->str()));
	}
	if(*dst != Variable::ATTR)
	{
		INTERNALERR(format("'{}': requires ATTR({}) for invocation target resolution", dst->TypeStr(), (int)Variable::ATTR));
	}

	_roff = ivk.dst + 1;

	auto& owner = dst->attr().owner;
	if(owner == Variable::CLASSOBJ)
	{
		auto cls = owner.clsObj()._cls;
		auto found = cls->_funcMap.find(dst->attr().name);
		if(found == cls->_funcMap.end())
		{//TODO
			//qaz
			throw RuntimeError::NoMember(owner._type, cls->name, dst->attr().name);
		}

		_clsStack.push(&owner);
		_prgStack.push(owner.clsObj()._prgObj);
		Exec(cls->_funcs[found->second], 1);
		_prgStack.pop();
		_clsStack.pop();

		auto vs = ResolveVar(ERefKind::Reg, ivk.dst + 1);
		auto vt = ResolveVar(ERefKind::Reg, ivk.dst);
		vt->SetVar(*vs);
		return true;
	}
	else if(owner == Variable::CLASS)
	{//TODO
		INTERNALERR(format("'{}': not implemented", owner.TypeStr()));
	}
	else if(owner == Variable::PROGRAMOBJ)
	{
		auto prg = owner.prgObj()._prg;
		auto found = prg->_classTable.find(dst->attr().name);
		if(found != prg->_classTable.end())
		{
			if(ivk.numArgs)
				_roff = ivk.dst + ivk.numArgs;
			else
				_roff = ivk.dst + 1;

			_prgStack.push(&owner);
			CreateClassObj(found->second, ivk.numArgs);
			_prgStack.pop();

			auto vs = ResolveVar(ERefKind::Reg, ivk.dst + 1);
			auto vt = ResolveVar(ERefKind::Reg, ivk.dst);
			vt->SetVar(*vs);
		}
		else
		{
			auto found = prg->_globalTable.find(dst->attr().name);
			if(found == prg->_globalTable.end())
			{//TODO
				//qaz;
				throw RuntimeError::NoMember(owner._type, prg->_name, dst->attr().name);
			}

			if(found->second.kind == EGlobalSymbol::Fn)
			{
				_roff = ivk.dst + 1;
				_prgStack.push(&owner);
				Exec(prg->_mainCode, found->second.pos);
				_prgStack.pop();

				auto vs = ResolveVar(ERefKind::Reg, ivk.dst + 1);
				auto vt = ResolveVar(ERefKind::Reg, ivk.dst);
				vt->SetVar(*vs);
			}
			else
			{//TODO
				INTERNALERR(format("'{}': incorrect kind", (int)found->second.kind));
			}
		}
		return true;
	}
	else if(owner == Variable::PROGRAM)
	{//TODO
		INTERNALERR(format("'{}': not implemented", owner.TypeStr()));
	}

	const ymod::ModuleDesc* modDesc = primitive::GetModuleDesc(owner._type);
	if(!modDesc)
	{
		if(owner != Variable::MODULE && owner != Variable::MODULEOBJ)
		{//TODO
			// qaz
			throw RuntimeError::NoMember(owner._type, "", dst->attr().name);
		}
		modDesc = owner.modObj()._mod.modDesc;
	}

	auto found = modDesc->memberTbl.find(dst->attr().name);
	if(found == modDesc->memberTbl.end())
	{//TODO
		//qaz
		throw RuntimeError::NoMember(owner._type, modDesc->name, dst->attr().name);
	}

	if(ivk.numArgs < found->second.numPrms)
	{//TODO qaz
		throw RuntimeError::NotMatchedParams(owner._type, modDesc->name, dst->attr().name, found->second.numPrms, ivk.numArgs);
	}

	YArgsAuto yaa;
	YArgs& ya = yaa._ya;
	int off = 0;
	if(found->second.needSelf || owner == Variable::MODULEOBJ)
	{
		if(owner == Variable::MODULE)
		{//TODO
			INTERNALERR(format("'{}': not implemented", owner.TypeStr()));
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
			auto arg = ResolveVar(ERefKind::Reg, argsRoff+i);
			YArg yo { (void*)arg, YEArg::YVar };
			ya.args[i+off] = yo;
		}
	}
	else
	{
		for(int i=0; i<ivk.numArgs; i++)
		{//TODO int value check
			auto arg = ResolveVar(ERefKind::Reg, argsRoff+i);
		}
	}

	auto ret = ResolveVar(ERefKind::Reg, ivk.dst);
	ya.retBuff.tp = YEArg::YVar;
	ya.retBuff.o = ret;

	auto yr = found->second.func(&ya);
	if(yr.code)
	{//TODO
		INTERNALERR(format("'{}::{}()': module logic error", modDesc->name, dst->attr().name));
	}
	if(modDesc->builtin)
	{
		if(yr.single.tp != YEArg::None)
		{
			if(yr.single.tp == YEArg::Object)
			{//TODO qaz
				YObj* yo = (YObj*)yr.single.o;
				ret->SetModule( _modMgr.GetModuleDesc(yo->name.str), true);
				ret->_type = Variable::MODULEOBJ;
				ret->modObj()._o = yo->obj;
				delete yo;
			}
			else if(yr.single.tp != YEArg::YVar)
			{//TODO
				INTERNALERR(format("'{}': not implemented", (int)yr.single.tp));
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

	ya.Reset(0);
	return true;
}

bool Machine::Inc(const Op::Inc& inc)
{
	Variable* name = ResolveVar(ERefKind::Const, inc.inc);

	if(!name)
		INTERNALERR(format("'constIdx:{}': include name not found", inc.inc));

	if(*name != Variable::STR)
		INTERNALERR(format("'constIdx:{}': incorrect include name type({})", inc.inc, name->TypeStr()));

	auto found = _prg->_programTable.find(name->str());
	if(found != _prg->_programTable.end())
	{
		auto& prg = found->second;
		//prg._mainCode
		auto v = ResolveVar((ERefKind)inc.dstKind, inc.dst);
		v->SetProgram(prg, true);

		_prgStack.push(v);

		int roffbk = _roff;
		_roff++;
		Exec(v->prgObj()._prg->_mainCode, 0);
		_roff = roffbk;

		_prgStack.pop();
	}
	else
	{
		const ymod::ModuleDesc& modDesc = _modMgr.GetModuleDesc(name->str());
		if(modDesc.IsNull())
		{//TODO
			INTERNALERR(format("'{}': module not found", name->str()));
		}

		auto v = ResolveVar((ERefKind)inc.dstKind, inc.dst);
		v->SetModule(modDesc, false);
	}
	return true;
}

bool Machine::Jnz(const Op::Jnz& jnz)
{
	Variable* test = ResolveVar((ERefKind)jnz.testKind, jnz.test);
	if(test->IsNullOrFalse())
		return true;

	_pc = jnz.pos - 1;
	return true;
}

bool Machine::NewMod(const Op::NewMod& nm)
{
	return false;
}

bool Machine::NewCls(const Op::NewCls& nc)
{
	Variable* dst = ResolveVar((ERefKind)nc.dstKind, nc.dst);
	if(*dst != Variable::STR)
	{//TODO
		INTERNALERR(format("'{}': incorrect class name type", dst->TypeStr()));
	}

	const Program* prg = _prgStack.top()->prgObj()._prg;


	auto found = prg->_classTable.find(dst->str());
	if(found == prg->_classTable.end())
	{//TODO
		INTERNALERR(format("'{}': class not found", dst->str()));
	}

	CreateClassObj(found->second, nc.numArgs);
	return true;
}

bool Machine::LValueField(const Op::LValueField& lvf)
{
	Variable* fld = ResolveVar((ERefKind)lvf.fieldKind, lvf.field);
	Variable* dst = ResolveVar((ERefKind)lvf.dstKind, lvf.dst);

	if(*fld != Variable::STR)
	{//TODO
		INTERNALERR(format("'{}': incorrect field name type", fld->TypeStr()));
	}

	if(*dst == Variable::PROGRAMOBJ)
	{
		auto& prg = *dst->prgObj()._prg;
		auto found = prg._globalTable.find(fld->str());
		if(found == prg._globalTable.end())
		{//TODO
			INTERNALERR(format("'{}': field not found in program({})", fld->str(), prg._name));
		}
		if(found->second.kind != EGlobalSymbol::Var)
		{//TODO
			INTERNALERR(format("'{}({})': incorrect field type", fld->str(), (int)found->second.kind));
		}

		dst->SetVarLVRef(*dst->prgObj()._globals.Get(found->second.idx));
	}
	else if(*dst == Variable::CLASSOBJ)
	{
		auto found = dst->clsObj()._cls->_fieldMap.find(fld->str());
		if(found == dst->clsObj()._cls->_fieldMap.end())
		{//TODO
			//qaz
			throw RuntimeError::NoMember(dst->_type, dst->clsObj()._cls->name, fld->str());
		}

		dst->SetVarLVRef(dst->clsObj()._fields[found->second], *dst);
	}
	else
	{//TODO
		INTERNALERR(format("'{}': unsupported owner type for '{}'", dst->TypeStr(), fld->str()));
	}

	return true;
}


bool Machine::CallBuiltinFunc(const Op::Call& cal)
{//TODO refactor
	/*if(cal.numPrms)
	{
		_roff -= (int)cal.numPrms - 1;
	}*/
	if((ERefKind)cal.dstKind != ERefKind::Reg)
	{//TODO check
		INTERNALERR(format("'{}': A return value must be assigned to a register({})", cal.dstKind, (int)ERefKind::Reg));
	}
	_roff = cal.dst;

	switch(cal.pos)
	{
	case 0xFFFF0000:
		if(cal.numPrms == 0)
		{//noop
		}
		else
		{
			auto v = ResolveVar(ERefKind::Reg, _roff);
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
			auto v = ResolveVar(ERefKind::Reg, _roff);
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
			auto v = ResolveVar(ERefKind::Reg, _roff);
			_retCode = (int)v->int_();
		}
		break;

	case 0xFFFF0000 + 3:
		{
			auto v = ResolveVar(ERefKind::Reg, _roff);
			v->SetStr("");
			getline(cin, *v->_u._s);
		}
		break;

	default:
		INTERNALERR(format("'{:X}': not registered builtin function", cal.pos));
	}

	return true;
}


bool Machine::CreateClassObj(const Class& cls, int numArgs)
{// TODO .. very fragile
	Variable v;
	v.SetClass(cls, true, _prgStack.top());

	_clsStack.push(&v);
	_prgStack.push(v.clsObj()._prgObj);

	int roffbk = _roff;
	_roff++;
	Exec(v.clsObj()._cls->_initer, 0);
	_roff = roffbk;

	if(numArgs)
	{
		_roff -= (int)numArgs - 1;
	}

	if(!v.clsObj()._cls->_ctor.empty())
	{
		Exec(v.clsObj()._cls->_ctor, 1);
	}

	_prgStack.pop();
	_clsStack.pop();

	auto dst = ResolveVar(ERefKind::Reg, _roff);
	dst->SetVar(v);
	return true;
}

int Machine::Run(const Program& program, int start /* = 0 */)
{
	_prg = &program;
	_retCode = INT_MAX;

	_prgObj.SetProgram(program, true);
	_prgStack.push(&_prgObj);
	try
	{
		Exec(_prg->_mainCode, start);
	}
	catch(RuntimeError e)
	{
		e._srcPath = _prgStack.top()->prgObj()._prg->_path;
		cout << "\n" << e.ToStr() << "\n";
		_retCode = -1;
	}
	_prgStack.pop();

	while(!_prgStack.empty()) _prgStack.pop();
	while(!_clsStack.empty()) _clsStack.pop();
	while(!_retStack.empty()) _retStack.pop();
	while(!_roffStack.empty()) _roffStack.pop();
	while(_rpStack.size() > 1) _rpStack.pop();
	_roff = 0;

	if(_retCode == INT_MAX) _retCode = 0;
	return _retCode;
}

int Machine::Continue(int start /* = -1 */)
{
	_retCode = INT_MAX;

	_prgStack.push(&_prgObj);
	try
	{
		Exec(_prg->_mainCode, start > -1 ? start : _pc);
	}
	catch(RuntimeError e)
	{
		e._srcPath = _prgStack.top()->prgObj()._prg->_path;
		cout << "\n" << e.ToStr() << "\n";
		_retCode = -1;
	}
	_prgStack.pop();

	while(!_prgStack.empty()) _prgStack.pop();
	while(!_clsStack.empty()) _clsStack.pop();
	while(!_retStack.empty()) _retStack.pop();
	while(!_roffStack.empty()) _roffStack.pop();
	while(_rpStack.size() > 1) _rpStack.pop();
	_roff = 0;

	if(_retCode == INT_MAX) _retCode = 0;
	return _retCode;
}

}
