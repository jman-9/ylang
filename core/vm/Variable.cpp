#include "Variable.h"
#include <format>
using namespace std;


namespace yvm
{


Variable* Variable::NewNum(int64_t num)
{
	auto v = new Variable;
	v->SetNum(num);
	return v;
}

Variable* Variable::NewStr(const string& str)
{
	auto v = new Variable;
	v->SetStr(str);
	return v;
}

Variable* Variable::NewList(const vector<Variable*>& list /*= std::vector<Variable>()*/)
{
	auto v = new Variable;
	v->SetList(list);
	return v;
}


void Variable::Clear()
{
	type = NONE;
	_int = 0;
	str = "";
	_float = 0.0;
	obj = nullptr;
	list = nullptr;
	dict = nullptr;
	ref = nullptr;
	attr = nullptr;
}

void Variable::SetNum(int64_t argNum)
{
	Clear();
	_int = argNum;
	type = INT;
}

void Variable::SetStr(const string& argStr)
{
	Clear();
	str = argStr;
	type = STR;
}

void Variable::SetList(const vector<Variable *>& argList /* = std::vector<Variable *>() */)
{
	Clear();
	type = LIST;
	list = new vector<Variable*>(argList);
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
				if(type == INT)
				{
					Clear();
					type = STR;
					str = format("{}{}", _int, rval.str);
				}
				else if(type == FLOAT)
				{
					Clear();
					type = STR;
					str = format("{}{}", _float, rval.str);
				}
				else if(type == STR)
				{//TODO
					if(rval.type == INT)
						str = to_string(rval._int);
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
			if(type == INT)
			{
				type = FLOAT;
				_float = (double)_int;
			}

			double rfloat = rval.type == FLOAT ? rval._float : (double)rval._int;

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
				type = INT;
				_int = 0;
			}

			switch(op)
			{
			case EToken::PlusAssign:	_int += rval._int; break;
			case EToken::MinusAssign:	_int -= rval._int; break;
			case EToken::MulAssign:		_int *= rval._int; break;
			case EToken::DivAssign:		_int /= rval._int; break;
			case EToken::ModAssign:		_int %= rval._int; break;
			case EToken::AndAssign:		_int &= rval._int; break;
			case EToken::OrAssign:		_int |= rval._int; break;
			case EToken::XorAssign:		_int ^= rval._int; break;
			case EToken::LShiftAssign:	_int <<= rval._int; break;
			case EToken::RShiftAssign:	_int >>= rval._int; break;
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
			if(lhs.type == INT)
			{
				str = format("{}{}", lhs._int, rhs.str);
			}
			else if(lhs.type == FLOAT)
			{
				str = format("{}{}", lhs._float, rhs.str);
			}
			else if(lhs.type == STR)
			{//TODO impl
				if(rhs.type == INT)
					str = lhs.str + to_string(rhs._int);
				else if(rhs.type == FLOAT)
					str = lhs.str + to_string(rhs._float);
				else if(rhs.type == STR)
					str = lhs.str + rhs.str;
				else
					throw 'n';	//TODO impl
			}
			type = STR;
		}
		else if(calcOp == EToken::Equal || calcOp == EToken::NotEqual)
		{
			if(lhs == STR && rhs == STR)
			{
				switch(calcOp)
				{
				case EToken::Equal:			_int = lhs.str == rhs.str; break;
				case EToken::NotEqual:		_int = lhs.str != rhs.str; break;
				}
			}
			else
			{
				type = INT;
				_int = 0;
			}
		}
		else if(calcOp == EToken::Dot)
		{
			if(rhs.type != STR)
			{
				throw 'n';
			}

			Attribute* newAttr = new Attribute();
			newAttr->owner = lhs;
			newAttr->name = rhs.str;
			Clear();
			type = ATTR;
			attr = newAttr;
		}
		else
		{//TODO impl
			throw 'n';
			return false;
		}
	}
	else if(lhs.type == FLOAT || rhs.type == FLOAT)
	{
		double lfloat = lhs.type == FLOAT ? lhs._float : (double)lhs._int;
		double rfloat = rhs.type == FLOAT ? rhs._float : (double)rhs._int;

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
				type = INT;
				switch(calcOp)
				{
				case EToken::And:			_int = lfloat && rfloat; break;
				case EToken::Or:			_int = lfloat || rfloat; break;
				case EToken::Greater:		_int = lfloat > rfloat; break;
				case EToken::Less:			_int = lfloat < rfloat; break;
				case EToken::GreaterEqual:	_int = lfloat >= rfloat; break;
				case EToken::LessEqual:		_int = lfloat <= rfloat; break;
				case EToken::Equal:			_int = lfloat == rfloat; break;
				case EToken::NotEqual:		_int = lfloat != rfloat; break;
				default:
					throw 'n';
				}
			}
		}
		type = FLOAT;
	}
	else
	{
		int64_t leftInt = lhs._int;
		int64_t rightInt = rhs._int;

		if(calcOp == EToken::Slash && rightInt == 0)
		{//TODO div 0
			throw 'n';
		}

		switch(calcOp)
		{
		case EToken::Plus:			_int = leftInt + rightInt; break;
		case EToken::Minus:			_int = leftInt - rightInt; break;
		case EToken::Star:			_int = leftInt * rightInt; break;
		case EToken::Slash:			_int = leftInt / rightInt; break;
		case EToken::Percent:		_int = leftInt % rightInt; break;
		case EToken::And:			_int = (leftInt != 0) && (rightInt != 0); break;
		case EToken::Or:			_int = (leftInt != 0) || (rightInt != 0); break;
		case EToken::Greater:		_int = leftInt > rightInt; break;
		case EToken::Less:			_int = leftInt < rightInt; break;
		case EToken::GreaterEqual:	_int = leftInt >= rightInt; break;
		case EToken::LessEqual:		_int = leftInt <= rightInt; break;
		case EToken::Equal:			_int = leftInt == rightInt; break;
		case EToken::NotEqual:		_int = leftInt != rightInt; break;
		case EToken::Amp:			_int = leftInt & rightInt; break;
		case EToken::Pipe:			_int = leftInt | rightInt; break;
		case EToken::Caret:			_int = leftInt ^ rightInt; break;
		case EToken::LShift:		_int = leftInt << rightInt; break;
		case EToken::RShift:		_int = leftInt >> rightInt; break;
		default:
			throw 'n';
		}
		type = INT;
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
	case EToken::UnaryPlus: _int = +rhs._int; break;
	case EToken::UnaryMinus: _int = -rhs._int; break;
	case EToken::Not: _int = (int64_t)(!rhs._int); break;
	case EToken::Tilde: _int = ~rhs._int; break;
	default:
		throw 'n';
	}
	type = INT;

	return true;
}

string Variable::ToStr() const
{
	switch(type)
	{
	case INT:
		return to_string(_int);
	case FLOAT:
		return to_string(_float);
	case STR:
		return str;
	case OBJECT:
		return "obj(wip)";
	case LIST:
		{
			string r = "[";
			if(!list->empty())
			{
				for(auto v: *list)
				{
					string t = v->ToStr();
					if(*v == STR) t = "'" + t + "'";
					r += t + ", ";
				}
				r.pop_back();
				r.pop_back();
			}
			r += "]";
			return r;
		}
	case DICT:
		{
			string r = "{";
			if(!dict->empty())
			{
				for(auto [k, v]: *dict)
				{
					string t = v->ToStr();
					if(*v == STR) t = "'" + t + "'";
					r += format("'{}': {}, ", k, t);
				}
				r.pop_back();
				r.pop_back();
			}
			r += "}";
			return r;
		}
	case REF:
		return "ref: " + ref->ToStr();
	case ATTR:
		return "attr(wip): " + attr->name;
	}
	return "";
}

bool Variable::operator==(Type cmp) const
{
	return type == cmp;
}
bool Variable::operator!=(Type cmp) const
{
	return type != cmp;
}

}
