#include "Machine.h"
#include "primitives/Primitives.h"
#include "builtin/BuiltinGarage.h"
#include <format>
#include <iostream>
using namespace std;


namespace yvm
{

Machine::Machine()
{
	_rp = 0;
	_sp = 0;
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
	case ERefKind::Const: return &_consts[idx];
	default: return nullptr;
	}
}


void Machine::Run(const Bytecode& code, int start /* = 0 */)
{
	_consts.clear();
	for(auto& c : code._consts)
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

	int i;
	for(i=start; i<code._code.size(); )
	{
		auto& inst = code._code[i];
		if(inst == EOpcode::Assign)
		{
			const Op::Assign& as = *(Op::Assign*)inst.code.data();

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
					dst->CalcAndAssign(*src1, (EToken)as.op, *src2);
				}
				else if(src1)
				{
					dst->Assign(EToken::Assign, *src1);
				}
				else if(src2)
				{
					dst->CalcUnaryAndAssign((EToken)as.op, *src2);
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

					Variable* src = ResolveVar((ERefKind)as.src2Kind, as.src2);
					Variable* dst = ResolveVar((ERefKind)as.src1Kind, as.src1);
					dst->Assign((EToken)as.op, *src);
				}
				else
				{//TODO func call, unary
					throw 'n';
				}
			}
		}
		else if(inst == EOpcode::PushSp)
		{
			_spStack.push(_sp);
		}
		else if(inst == EOpcode::PopSp)
		{
			_sp = _spStack.top();
			_spStack.pop();
		}
		else if(inst == EOpcode::Jmp)
		{
			const Op::Jmp& jmp = *(Op::Jmp*)inst.code.data();
			i = jmp.pos;
			continue;
		}
		else if(inst == EOpcode::Invoke)
		{
			const Op::Invoke& ivk = *(Op::Invoke*)inst.code.data();

			_rp -= (int)ivk.numPrms;
			_rpStack.push(_rp);

			if(ivk.pos >= 0xFFFF0000)
			{// todo new architecture
				if(ivk.pos == 0xFFFF0000)
				{
					if(ivk.numPrms == 0)
					{//noop
					}
					else
					{
						auto v = ResolveVar(ERefKind::Reg, 0);
						cout << v->ToStr();
					}
				}
				else if(ivk.pos == 0xFFFF0000 + 1)
				{
					if(ivk.numPrms == 0)
					{
						cout << "\n";
					}
					else
					{
						auto v = ResolveVar(ERefKind::Reg, 0);
						cout << v->ToStr() << "\n";
					}
				}
				else
				{
					throw 'n';
				}
				_rp = _rpStack.top();
				_rpStack.pop();
			}
			else
			{
				_retStack.push((uint32_t)i+1);
				_cspStack.push(_sp);

				i = ivk.pos;
				continue;
			}
		}
		else if(inst == EOpcode::Ret)
		{
			_sp = _cspStack.top();
			_cspStack.pop();
			_rp = _rpStack.top() + 1;
			_rpStack.pop();
			i = _retStack.top();
			_retStack.pop();
			continue;
		}
		else if(inst == EOpcode::Jz)
		{
			const Op::Jz& jz = *(Op::Jz*)inst.code.data();
			Variable* test = ResolveVar((ERefKind)jz.testKind, jz.test);
			if(!test->_int)
			{
				i = jz.pos;
				continue;
			}
		}
		else if(inst == EOpcode::ListSet)
		{
			const Op::ListSet& ls = *(Op::ListSet*)inst.code.data();
			Variable* dst = ResolveVar((ERefKind)ls.dstKind, ls.dst);

			Variable* t = nullptr;
			if(dst->_type != Variable::REF)
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
		}
		else if(inst == EOpcode::ListAdd)
		{
			const Op::ListAdd& la = *(Op::ListAdd*)inst.code.data();
			Variable* src = ResolveVar((ERefKind)la.srcKind, la.src);
			Variable* dst = ResolveVar((ERefKind)la.dstKind, la.dst);
			if(dst->_type != Variable::LIST)
			{
				throw 'n';
			}
			dst->_list->push_back(src->Clone());
		}
		else if(inst == EOpcode::DictSet)
		{
			const Op::DictSet& ds = *(Op::DictSet*)inst.code.data();
			Variable* dst = ResolveVar((ERefKind)ds.dstKind, ds.dst);

			Variable* t = nullptr;
			if(dst->_type != Variable::REF)
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
		}
		else if(inst == EOpcode::DictAdd)
		{
			const Op::DictAdd& da = *(Op::DictAdd*)inst.code.data();
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
		}
		else if(inst == EOpcode::Index)
		{
			const Op::Index& li = *(Op::Index*)inst.code.data();
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
		}
		else if(inst == EOpcode::LValueIndex)
		{
			const Op::LValueIndex& lli = *(Op::LValueIndex*)inst.code.data();
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
				dst->_type = Variable::REF;
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
				dst->_type = Variable::REF;
				dst->_ref = t;
			}
			else
			{
				throw 'n';
			}
		}
		else if(inst == EOpcode::Call)
		{
			const Op::Call& cal = *(Op::Call*)inst.code.data();

			_rp = _rp - cal.numArgs - 1;
			_rpStack.push(_rp);

			Variable* dst = ResolveVar((ERefKind)cal.dstKind, cal.dst);
			if(*dst == Variable::STR)
			{
				const auto& mod = _modMgr.GetModule(dst->_str);
				if(mod.IsNull())
				{//TODO
					throw 'n';
				}
				if(!mod.issuer)
				{//TODO
					throw 'n';
				}

				YRet yr = mod.issuer(nullptr);
				if(yr.single.tp != YEObj::License)
				{
					throw 'n';
				}

				_rpStack.pop();
				auto v = ResolveVar(ERefKind::Reg, 0);
				v->Clear();
				v->_type = Variable::LICENSE;
				v->_obj = yr.single.o;
				v->_mod = mod;
				i++;
				continue;
			}
			if(dst->_type != Variable::ATTR)
			{
				throw 'n';
			}

			auto& owner = dst->_attr->owner;
			const ymod::Module* mod = primitive::GetModule(owner._type);
			if(!mod)
			{
				if(owner != Variable::MODULE && owner != Variable::LICENSE)
				{//TODO
					throw 'n';
				}
				mod = &owner._mod;
			}

			auto found = mod->funcTbl.find(dst->_attr->name);
			if(found == mod->funcTbl.end())
			{//TODO
				throw 'n';
			}

			if(cal.numArgs < found->second.numPrms)
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
				ya.Reset(found->second.numPrms + 1);
				if(owner == Variable::LICENSE)
				{
					ya.args[0] = owner.ToContract();
				}
				else
				{
					if(mod->builtin)
					{
						ya.args[0].tp = YEObj::YVar;
						ya.args[0].o = &owner;
					}
					else
					{//TODO
						throw 'n';
					}
				}
			}
			else
			{
				off = 0;
				ya.Reset(found->second.numPrms);
			}

			if(mod->builtin)
			{
				for(int i=0; i<found->second.numPrms; i++)
				{//TODO int value check
					auto arg = ResolveVar(ERefKind::Reg, i+1);
					YObj yo { (void*)arg, YEObj::YVar };
					ya.args[i+off] = yo;
				}
			}
			else
			{
				for(int i=0; i<found->second.numPrms; i++)
				{//TODO int value check
					auto arg = ResolveVar(ERefKind::Reg, i+1);
					ya.args[i+off] = arg->ToContract();
				}
			}

			Variable* ret = nullptr;
			auto yr = found->second.func(&ya);
			if(yr.code)
			{//TODO
				throw 'n';
			}
			if(mod->builtin)
			{
				if(yr.single.tp != YEObj::None)//TODO real val
				{
					ret = (Variable*)yr.single.o;
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

			_rpStack.pop();
			auto v = ResolveVar(ERefKind::Reg, 0);
			*v = ret ? *ret : Variable();
		}
		else if(inst == EOpcode::Inc)
		{
			const Op::Inc& inc = *(Op::Inc*)inst.code.data();
			Variable* name = ResolveVar(ERefKind::Const, inc.inc);

			if(!name || *name != Variable::STR)
			{
				throw 'n';
			}

			const ymod::Module& mod = _modMgr.GetModule(name->_str);
			if(mod.IsNull())
			{//TODO
				throw 'n';
			}

			auto v = ResolveVar(ERefKind::LocalVar, _sp);
			v->SetModule(mod);
		}

		i++;
	}
}

}
