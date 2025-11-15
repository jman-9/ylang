#include "Variable.h"
#include <format>
using namespace std;


namespace yvm
{


Variable* Variable::NewNum(int64_t num)
{
	auto v = new Variable;
	v->SetInt(num);
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
yvm::Variable* Variable::New(YObj o)
{
	auto v = new Variable;
	v->SetValueFromContract(o);
	return v;
}


void Variable::Clear()
{
	_type = NONE;
	_int = 0;
	_str = "";
	_float = 0.0;
	_obj = nullptr;
	_list = nullptr;
	_dict = nullptr;
	_ref = nullptr;
	_attr = nullptr;
}

void Variable::SetInt(int64_t argNum)
{
	Clear();
	_int = argNum;
	_type = INT;
}
void Variable::SetFloat(double f)
{
	Clear();
	_float = f;
	_type = FLOAT;
}
void Variable::SetStr(const string& argStr)
{
	Clear();
	_str = argStr;
	_type = STR;
}
void Variable::SetList(const vector<Variable *>& argList /* = std::vector<Variable *>() */)
{
	Clear();
	_type = LIST;
	_list = new vector<Variable*>(argList);
}
void Variable::SetModule(const ymod::Module& mod)
{
	Clear();
	_type = MODULE;
	_mod = mod;
}
void Variable::SetValueFromContract(YObj o)
{
	switch(o.tp)
	{
	case YEObj::Int64: return SetInt(o.ToInt64());
	case YEObj::Double: return SetFloat(o.ToDouble());
	case YEObj::Str: return SetStr(o.ToStr());
	default://TODO
		throw 'n';
	}
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

	if(_type == REF)
	{
		auto t = _ref;
		Clear();
		return t->Assign(op, rval);
	}

	if(op == EToken::Assign)
	{
		*this = rval;
	}
	else
	{
		if(_type == LIST || rval._type == LIST)
		{
			if(_type != LIST || rval._type != LIST)
			{
				throw 'n';
				return false;
			}

			int a = 1;
		}
		else if(_type == STR || rval._type == STR)
		{
			if(op == EToken::PlusAssign)
			{
				if(_type == INT)
				{
					Clear();
					_type = STR;
					_str = format("{}{}", _int, rval._str);
				}
				else if(_type == FLOAT)
				{
					Clear();
					_type = STR;
					_str = format("{}{}", _float, rval._str);
				}
				else if(_type == STR)
				{
					_str += rval.ToStr();
				}
			}
			else
			{//TODO impl
				throw 'n';
				return false;
			}
		}
		else if(_type == FLOAT || rval._type == FLOAT)
		{
			if(_type == INT)
			{
				_type = FLOAT;
				_float = (double)_int;
			}

			double rfloat = rval._type == FLOAT ? rval._float : (double)rval._int;

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
			if(_type == NONE)
			{
				_type = INT;
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

	if(lhs._type == STR || rhs._type == STR)
	{//todo refactor
		if(calcOp == EToken::Plus)
		{
			if(lhs._type == INT)
			{
				_str = format("{}{}", lhs._int, rhs._str);
			}
			else if(lhs._type == FLOAT)
			{
				_str = format("{}{}", lhs._float, rhs._str);
			}
			else if(lhs._type == STR)
			{
				_str = lhs._str + rhs.ToStr();
			}
			_type = STR;
		}
		else if(calcOp == EToken::Equal || calcOp == EToken::NotEqual)
		{
			if(lhs == STR && rhs == STR)
			{
				switch(calcOp)
				{
				case EToken::Equal:			_int = lhs._str == rhs._str; break;
				case EToken::NotEqual:		_int = lhs._str != rhs._str; break;
				}
			}
			else
			{
				_type = INT;
				_int = 0;
			}
		}
		else if(calcOp == EToken::Dot)
		{
			if(rhs._type != STR)
			{
				throw 'n';
			}

			Attribute* newAttr = new Attribute();
			newAttr->owner = lhs;
			newAttr->name = rhs._str;
			Clear();
			_type = ATTR;
			_attr = newAttr;
		}
		else
		{//TODO impl
			throw 'n';
			return false;
		}
	}
	else if(lhs._type == FLOAT || rhs._type == FLOAT)
	{
		double lfloat = lhs._type == FLOAT ? lhs._float : (double)lhs._int;
		double rfloat = rhs._type == FLOAT ? rhs._float : (double)rhs._int;

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
				_type = INT;
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
		_type = FLOAT;
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
		_type = INT;
	}

	return true;
}

bool Variable::CalcUnaryAndAssign(EToken unaryOp, const Variable& rhs)
{
	if(rhs._type == STR || rhs._type == NONE)
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
	_type = INT;

	return true;
}

YObj Variable::ToContract() const
{
	auto ToYStr = [](const string& s) -> YStr*
	{
		YStr* ys = new YStr;
		ys->str = new char[s.size()];
		memcpy(ys->str, s.data(), s.size());
		ys->len = (int)s.size();
		return ys;
	};
	auto ToList = [this]() -> YList*
	{
		YList* yl = new YList;
		yl->sz = (int)_list->size();
		yl->list = new YObj[yl->sz];
		for(int i=0; i<yl->sz; i++)
		{
			yl->list[i] = _list->at(i)->ToContract();
		}
		return yl;
	};
	auto ToDict = [this, ToYStr]() -> YDict*
	{
		YDict* yd = new YDict;
		yd->sz = (int)_dict->size();
		yd->keys = new YObj[yd->sz];
		yd->vals = new YObj[yd->sz];
		auto it = _dict->begin();
		for(int i=0; i<yd->sz; i++, it++)
		{
			yd->keys[i] = { (void*)ToYStr(it->first), YEObj::Str };
			yd->vals[i] = it->second->ToContract();
		}
		return yd;
	};

	switch(_type)
	{
	case INT: return { (void*)(intptr_t)_int, YEObj::Int64 };
	case FLOAT: return { (void*)(*(intptr_t*)&_float), YEObj::Double };
	case STR: return { (void*)ToYStr(_str), YEObj::Str };
	case LIST: return { (void*)ToList(), YEObj::List };
	case DICT: return { (void*)ToDict(), YEObj::Dict };
	case LICENSE: return { _obj, YEObj::License };
	default: throw 'n';
	}
	return YObj();
}

string Variable::ToStr() const
{
	switch(_type)
	{
	case INT:
		return to_string(_int);
	case FLOAT:
		return to_string(_float);
	case STR:
		return _str;
	case OBJECT:
		return "obj(wip)";
	case LIST:
		{
			string r = "[";
			if(!_list->empty())
			{
				for(auto v: *_list)
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
			if(!_dict->empty())
			{
				for(auto [k, v]: *_dict)
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
		return "ref: " + _ref->ToStr();
	case ATTR:
		return "attr(wip): " + _attr->name;
	}
	return "";
}

bool Variable::operator==(Type cmp) const
{
	return _type == cmp;
}
bool Variable::operator!=(Type cmp) const
{
	return _type != cmp;
}

}
