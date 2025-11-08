#include "Machine.h"
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
		_consts.push_back({ .type = (Variable::Type)c.type, .num = c.num, .str = c.str, ._float = c._float });
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
					auto v = ResolveVar(ERefKind::Reg, _rp);

					if(v->type == Variable::NUM)
					{
						cout << v->num;
					}
					else if(v->type == Variable::STR)
					{
						cout << v->str;
					}
					else if(v->type == Variable::LIST)
					{//TODO print list
						cout << "LIST";
					}
				}
				else if(ivk.pos == 0xFFFF0000 + 1)
				{
					auto v = ResolveVar(ERefKind::Reg, _rp);

					if(v->type == Variable::NUM)
					{
						cout << v->num << "\n";
					}
					else if(v->type == Variable::STR)
					{
						cout << v->str << "\n";
					}
					else if(v->type == Variable::LIST)
					{//TODO print list
						cout << "LIST" << "\n";
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
			if(!test->num)
			{
				i = jz.pos;
				continue;
			}
		}
		else if(inst == EOpcode::ListSet)
		{
			const Op::ListSet& ls = *(Op::ListSet*)inst.code.data();
			Variable* dst = ResolveVar((ERefKind)ls.dstKind, ls.dst);
			dst->Clear();
			dst->type = Variable::LIST;
		}
		else if(inst == EOpcode::ListAdd)
		{
			const Op::ListAdd& la = *(Op::ListAdd*)inst.code.data();
			Variable* src = ResolveVar((ERefKind)la.srcKind, la.src);
			Variable* dst = ResolveVar((ERefKind)la.dstKind, la.dst);
			if(dst->type != Variable::LIST)
			{
				throw 'n';
			}
			dst->arr.push_back(src->Clone());
		}
		else if(inst == EOpcode::Index)
		{
			const Op::Index& li = *(Op::Index*)inst.code.data();
			Variable* idx = ResolveVar((ERefKind)li.idxKind, li.idx);
			Variable* dst = ResolveVar((ERefKind)li.dstKind, li.dst);

			if(dst->type != Variable::LIST)
			{
				throw 'n';
			}
			if(idx->type != Variable::NUM)
			{
				throw 'n';
			}

			*dst = *dst->arr[idx->num];
		}
		else if(inst == EOpcode::LValueIndex)
		{
			const Op::LValueIndex& lli = *(Op::LValueIndex*)inst.code.data();
			Variable* idx = ResolveVar((ERefKind)lli.idxKind, lli.idx);
			Variable* dst = ResolveVar((ERefKind)lli.dstKind, lli.dst);

			if(dst->type != Variable::LIST)
			{
				throw 'n';
			}
			if(idx->type != Variable::NUM)
			{
				throw 'n';
			}

			auto t = dst->arr[idx->num];
			dst->Clear();
			dst->type = Variable::REF;
			dst->ref = t;
		}

		i++;
	}
}

}
