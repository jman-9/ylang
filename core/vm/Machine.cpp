#include "Machine.h"
#include "../Token.h"
#include <format>
#include <iostream>
using namespace std;

namespace yvm
{

void Variable::Clear()
{
	type = NONE;
	num = 0;
	str = "";
	_float = 0.0;
	obj = nullptr;
}

bool Variable::Assign(EToken op, const Variable& rval)
{
	if(!Token::IsAssign(op))
	{
		throw 'n';
		return false;
	}

	if(op == EToken::Assign)
	{
		*this = rval;
	}
	else
		{
		if(type == STR || rval.type == STR)
		{
			if(op == EToken::PlusAssign)
			{
				if(type == NUM)
				{
					Clear();
					type = STR;
					str = format("{}{}", num, rval.str);
				}
				else if(type == FLOAT)
				{
					Clear();
					type = STR;
					str = format("{}{}", _float, rval.str);
				}
				else if(type == STR)
				{//TODO
					if(rval.type == NUM)
						str = to_string(rval.num);
					else if(rval.type == Constant::FLOAT)
						str = to_string(rval._float);
					else if(rval.type == STR)
						str += rval.str;
					else
						throw 'n';	//TODO impl
				}
			}
			else
			{//TODO impl
				throw 'n';
				return false;
			}
		}
		else if(type == FLOAT || rval.type == FLOAT)
		{
			if(type == NUM)
			{
				type = FLOAT;
				_float = (double)num;
			}

			double rfloat = rval.type == FLOAT ? rval._float : (double)rval.num;

			switch(op)
			{
			case EToken::PlusAssign:	_float += rfloat; break;
			case EToken::MinusAssign:	_float -= rfloat; break;
			case EToken::MulAssign:		_float *= rfloat; break;
			case EToken::DivAssign:		_float /= rfloat; break;
			default: throw 'n';
			}
		}
		else
		{
			if(type == NONE)
			{
				type = NUM;
				num = 0;
			}

			switch(op)
			{
			case EToken::PlusAssign:	num += rval.num; break;
			case EToken::MinusAssign:	num -= rval.num; break;
			case EToken::MulAssign:		num *= rval.num; break;
			case EToken::DivAssign:		num /= rval.num; break;
			case EToken::ModAssign:		num %= rval.num; break;
			case EToken::AndAssign:		num &= rval.num; break;
			case EToken::OrAssign:		num |= rval.num; break;
			case EToken::XorAssign:		num ^= rval.num; break;
			case EToken::LShiftAssign:	num <<= rval.num; break;
			case EToken::RShiftAssign:	num >>= rval.num; break;
			}
		}
	}

	return true;
}

bool Variable::CalcAndAssign(const Variable& lhs, EToken calcOp, const Variable& rhs)
{
	//TODO +, - confusion if(Token::IsPrefixUnary(calcOp))

	if(lhs.type == STR || rhs.type == STR)
	{//todo refactor
		if(calcOp == EToken::Plus)
		{
			if(lhs.type == NUM)
			{
				str = format("{}{}", lhs.num, rhs.str);
			}
			else if(lhs.type == FLOAT)
			{
				str = format("{}{}", lhs._float, rhs.str);
			}
			else if(lhs.type == STR)
			{//TODO impl
				if(rhs.type == NUM)
					str = lhs.str + to_string(rhs.num);
				else if(rhs.type == Constant::FLOAT)
					str = lhs.str + to_string(rhs._float);
				else if(rhs.type == STR)
					str = lhs.str + rhs.str;
				else
					throw 'n';	//TODO impl
			}
			type = STR;
		}
		else
		{//TODO impl
			throw 'n';
			return false;
		}
	}
	else if(lhs.type == FLOAT || rhs.type == FLOAT)
	{
		double lfloat = lhs.type == FLOAT ? lhs._float : (double)lhs.num;
		double rfloat = rhs.type == FLOAT ? rhs._float : (double)rhs.num;

		switch(calcOp)
		{
		case EToken::Plus:			_float = lfloat + rfloat; break;
		case EToken::Minus:			_float = lfloat - rfloat; break;
		case EToken::Star:			_float = lfloat * rfloat; break;
		case EToken::Slash:			_float = lfloat / rfloat; break;
		default:
			{
				type = NUM;
				switch(calcOp)
				{
				case EToken::And:			num = lfloat && rfloat; break;
				case EToken::Or:			num = lfloat || rfloat; break;
				case EToken::Greater:		num = lfloat > rfloat; break;
				case EToken::Less:			num = lfloat < rfloat; break;
				case EToken::GreaterEqual:	num = lfloat >= rfloat; break;
				case EToken::LessEqual:		num = lfloat <= rfloat; break;
				case EToken::Equal:			num = lfloat == rfloat; break;
				case EToken::NotEqual:		num = lfloat != rfloat; break;
				default:
					throw 'n';
				}
			}
		}
		type = FLOAT;
	}
	else
	{
		int64_t lnum = lhs.num;
		int64_t rnum = rhs.num;

		switch(calcOp)
		{
		case EToken::Plus:			num = lnum + rnum; break;
		case EToken::Minus:			num = lnum - rnum; break;
		case EToken::Star:			num = lnum * rnum; break;
		case EToken::Slash:			num = lnum / rnum; break;
		case EToken::Percent:		num = lnum % rnum; break;
		case EToken::And:			num = (lnum != 0) && (rnum != 0); break;
		case EToken::Or:			num = (lnum != 0) || (rnum != 0); break;
		case EToken::Greater:		num = lnum > rnum; break;
		case EToken::Less:			num = lnum < rnum; break;
		case EToken::GreaterEqual:	num = lnum >= rnum; break;
		case EToken::LessEqual:		num = lnum <= rnum; break;
		case EToken::Equal:			num = lnum == rnum; break;
		case EToken::NotEqual:		num = lnum != rnum; break;
		case EToken::Amp:			num = lnum & rnum; break;
		case EToken::Pipe:			num = lnum | rnum; break;
		case EToken::Caret:			num = lnum ^ rnum; break;
		case EToken::LShift:		num = lnum << rnum; break;
		case EToken::RShift:		num = lnum >> rnum; break;
		default:
			throw 'n';
		}
		type = NUM;
	}

	return true;
}



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


void Machine::Run(const Bytecode& code)
{
	for(auto& c : code._consts)
	{
		_consts.push_back({ .type = (Variable::Type)c.type, .num = c.num, .str = c.str, ._float = c._float });
	}

	int i;
	for(i=0; i<code._code.size(); )
	{
		auto& inst = code._code[i];
		if(inst == EOpcode::Assign)
		{
			const Inst::Assign& as = *(Inst::Assign*)inst.code.data();

			if((ERefKind)as.dstKind == ERefKind::Const)
			{
				throw 'n';
			}

			if((ERefKind)as.dstKind != ERefKind::None)
			{
				Variable* dst = ResolveVar((ERefKind)as.dstKind, as.dst);
				Variable* src1 = ResolveVar((ERefKind)as.src1Kind, as.src1);
				Variable* src2 = ResolveVar((ERefKind)as.src2Kind, as.src2);
				if(src1 && src2)
				{
					dst->CalcAndAssign(*src1, (EToken)as.op, *src2);
				}
				else if(src1)
				{
					dst->Assign(EToken::Assign, *src1);
				}
				else
				{//TODO unary
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

					Variable* dst = ResolveVar((ERefKind)as.src1Kind, as.src1);
					Variable* src = ResolveVar((ERefKind)as.src2Kind, as.src2);
					dst->Assign((EToken)as.op, *src);
				}
				else
				{//TODO func call, unary
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
			const Inst::Jmp& jmp = *(Inst::Jmp*)inst.code.data();
			i = jmp.pos;
			continue;
		}
		else if(inst == EOpcode::Invoke)
		{
			const Inst::Invoke& ivk = *(Inst::Invoke*)inst.code.data();

			_rp -= (int)ivk.numPrms;
			_rpStack.push(_rp);

			if(ivk.pos >= 0xFFFF0000)
			{// todo new architecture
				if(ivk.pos == 0xFFFF0000)
				{
					auto v = ResolveVar(ERefKind::Reg, _rp);

					if(v->type == Variable::NUM)
					{
						cout << v->NUM;
					}
					else if(v->type == Variable::STR)
					{
						cout << v->STR << "\n";
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
			_rp = _rpStack.top();
			_rpStack.pop();
			i = _retStack.top();
			_retStack.pop();
			continue;
		}
		else if(inst == EOpcode::Jz)
		{
			const Inst::Jz& jz = *(Inst::Jz*)inst.code.data();
			Variable* test = ResolveVar((ERefKind)jz.testKind, jz.test);
			if(!test->num)
			{
				i = jz.pos;
				continue;
			}
		}

		i++;
	}

	int a = 1;
}


}
