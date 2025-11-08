#include "Variable.h"
#include <format>
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
	arr.clear();
	ref = nullptr;
}

Variable* Variable::Clone()
{
	auto v = new Variable();
	*v = *this;
	return v;
}

bool Variable::Assign(EToken op, const Variable& rval)
{
	if(!Token::IsAssign(op))
	{
		throw 'n';
		return false;
	}

	if(type == REF)
	{
		auto t = ref;
		Clear();
		return t->Assign(op, rval);
	}

	if(op == EToken::Assign)
	{
		*this = rval;
	}
	else
	{
		if(type == LIST || rval.type == LIST)
		{
			if(type != LIST || rval.type != LIST)
			{
				throw 'n';
				return false;
			}

			int a = 1;
		}
		else if(type == STR || rval.type == STR)
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
					else if(rval.type == FLOAT)
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
				else if(rhs.type == FLOAT)
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

		if(calcOp == EToken::Slash && rfloat == 0.0)
		{//TODO div 0
			throw 'n';
		}

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

		if(calcOp == EToken::Slash && rnum == 0)
		{//TODO div 0
			throw 'n';
		}

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

bool Variable::CalcUnaryAndAssign(EToken unaryOp, const Variable& rhs)
{
	if(rhs.type == STR || rhs.type == NONE)
	{
		throw 'n';
		return false;
	}

	switch(unaryOp)
	{
	case EToken::UnaryPlus: num = +rhs.num; break;
	case EToken::UnaryMinus: num = -rhs.num; break;
	case EToken::Not: num = (int64_t)(!rhs.num); break;
	case EToken::Tilde: num = ~rhs.num; break;
	default:
		throw 'n';
	}
	type = NUM;

	return true;
}

}
