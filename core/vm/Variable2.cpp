#include "Variable2.h"


namespace yvm
{
using namespace std;
using namespace ymod;

#define EPSILON (1e-9)


Variable2::Variable2()
{
	_type = NONE;
	_u._s = nullptr;
}
Variable2::Variable2(const Variable2& v)
{
	SetVar(*(Variable2*)&v);
}
Variable2::~Variable2()
{
	Clear();
}

void Variable2::Clear()
{
	switch(_type)
	{
	case STR:
		if(_u._s) { delete _u._s; _u._s = nullptr; }
		break;

	case ATTR:
		if(_u._attr) { delete _u._attr; _u._attr = nullptr; }
		break;

	case LIST:
	case DICT:
	case CLASS:
	case CLASSOBJ:
	case MODULEOBJ:
		if(_u._o) { _u._o->ReleaseRef(); _u._o = nullptr; }
		break;

	}

	_type = NONE;
}

void Variable2::SetInt(int64_t i)
{
	Clear();
	_u._i = i;
	_type = INT;
}
void Variable2::SetFloat(double f)
{
	Clear();
	_u._f = f;
	_type = FLOAT;
}
void Variable2::SetStr(string s)
{
	Clear();
	_u._s = new string(s);
	_type = STR;
}
void Variable2::SetObj(Object* obj)
{
	auto t = obj;
	obj->AddRef();

	Clear();
	_u._o = obj;
	_u._o->AddRef();
	_type = obj->_type;

	t->ReleaseRef();
}

void Variable2::SetValueFromContract(YArg o)
{
	switch(o.tp)
	{
	case YEArg::Int64: return SetInt(o.ToInt64());
	case YEArg::Double: return SetFloat(o.ToDouble());
	case YEArg::Str: return SetStr(o.ToStr());
	case YEArg::YVar: SetVar(*(Variable2*)o.o); break;
	default://TODO
		throw 'n';
	}
}

void Variable2::SetVarRef(Variable2& var)
{
	Clear();
	_u._ref = &var;
	_type = REF;

}
void Variable2::SetVarLVRef(Variable2& var)
{
	Clear();
	_u._ref = &var;
	_type = LVREF;
}
void Variable2::SetAttr(Variable2& owner, string name)
{
	auto t = new Attribute2{owner, name};
	Clear();
	_u._attr = t;
	_type = ATTR;
}
void Variable2::SetAttr(Attribute2& attr)
{
	SetAttr(attr.owner, attr.name);
}
void Variable2::SetList(const std::vector<Variable2>& list /*= std::vector<Variable2>()*/)
{
	ResetNewRef();
	for(auto& e : list)
		_u._o->_list.push_back(e);
	_u._o->_type = LIST;
	_type = LIST;
}
void Variable2::SetDict(const std::unordered_map<std::string, Variable2>& dict /*= std::unordered_map<std::string, Variable2>()*/)
{	ResetNewRef();
	for(auto& e : dict)
		_u._o->_dict.insert(e);
	_u._o->_type = DICT;
	_type = DICT;
}
void Variable2::SetClass(const Class& cls, bool makeInstance)
{
	ResetNewRef();
	_u._o->_clso._cls = &cls;
	if(!makeInstance)	//TODO
		return;
	_u._o->_type = CLASS;
	_type = CLASS;

	clsObj()._fields.resize(clsObj()._cls->_fields.size());
}
void Variable2::SetModule(const ModuleDesc& mod, bool makeInstance)
{
	ResetNewRef();
	_u._o->_modo._mod.modDesc = &mod;
	_u._o->_type = MODULE;
	_type = MODULE;

	if(mod.initer)
	{
		modObj()._mod = mod.initer();
	}

	if(!makeInstance)	//TODO
		return;

	_u._o->_type = MODULEOBJ;
	_type = MODULEOBJ;
	/*TODO qaz
	YRet yr = mod.newer(nullptr);
	if(yr.single.tp != YEArg::Object)
	{
		throw 'n';
	}*/
}

void Variable2::SetVar(Variable2& var)
{
	if(this == &var)
	{//guard code against self-assignment
		return;
	}

	switch(var._type)
	{
	case NONE: Clear(); break;
	case INT: SetInt(var._u._i); break;
	case FLOAT: SetFloat(var._u._f); break;
	case STR: SetStr(*var._u._s); break;
	case REF: SetVarRef(var); break;
	case LVREF: SetVarLVRef(var); break;
	case ATTR: SetAttr(var._u._attr->owner, var._u._attr->name); break;

	case LIST:
	case DICT:
	case CLASS:
	case MODULE:
	case MODULEOBJ:
	case CLASSOBJ: SetObj(var._u._o); break;
	}
}

bool Variable2::Assign(EToken op, Variable2& rval)
{
	if(!Token::IsAssign(op))
	{
		throw 'n';
		return false;
	}

	if(_type == LVREF)
	{
		auto t = _u._ref;
		if(!t->Assign(op, rval))
		{
			throw 'n';
		}
		this->SetVar(*t);
		return true;
	}

	if(op == EToken::Assign)
	{
		this->SetVar(rval);
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
			//TODO
			int a = 1;
		}
		else if(_type == STR || rval._type == STR)
		{
			if(op == EToken::PlusAssign)
			{
				SetStr(format("{}{}", ToStr(), *rval._u._s));
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
				SetFloat((double)_u._i);
			}

			if(_type != FLOAT)
			{//TODO
				throw 'n';
				return false;
			}

			if(rval._type != FLOAT && rval._type != INT)
			{//TODO
				throw 'n';
				return false;
			}

			double rfloat = rval._type == FLOAT ? rval._u._f : (double)rval._u._i;

			switch(op)
			{
			case EToken::PlusAssign:	rval._u._f += rfloat; break;
			case EToken::MinusAssign:	rval._u._f -= rfloat; break;
			case EToken::MulAssign:		rval._u._f *= rfloat; break;
			case EToken::DivAssign:		rval._u._f /= rfloat; break;
			default: throw 'n';
			}
		}
		else if(_type == INT && rval._type == INT)
		{
			switch(op)
			{
			case EToken::PlusAssign:	_u._i += rval._u._i; break;
			case EToken::MinusAssign:	_u._i -= rval._u._i; break;
			case EToken::MulAssign:		_u._i *= rval._u._i; break;
			case EToken::DivAssign:		_u._i /= rval._u._i; break;
			case EToken::ModAssign:		_u._i %= rval._u._i; break;
			case EToken::AndAssign:		_u._i &= rval._u._i; break;
			case EToken::OrAssign:		_u._i |= rval._u._i; break;
			case EToken::XorAssign:		_u._i ^= rval._u._i; break;
			case EToken::LShiftAssign:	_u._i <<= rval._u._i; break;
			case EToken::RShiftAssign:	_u._i >>= rval._u._i; break;
			}
		}
		else
		{//TODO
			throw 'n';
			return false;
		}
	}

	return true;
}

bool Variable2::CalcAndAssign(Variable2& lhs, EToken calcOp, Variable2& rhs)
{
	//TODO +, - confusion if(Token::IsPrefixUnary(calcOp))

	if(_type == LVREF)
	{
		auto t = _u._ref;
		if(!t->CalcAndAssign(lhs, calcOp, rhs))
		{
			throw 'n';
		}
		SetVar(*t);
		return true;
	}

	if(lhs == _NULL_ || rhs == _NULL_ || lhs == NONE || rhs == NONE)
	{//todo refactor
		int lv = lhs == _NULL_ || lhs == NONE;
		int rv = rhs == _NULL_ || rhs == NONE;

		switch(calcOp)
		{
		case EToken::Equal:		SetInt(lv && rv); break;
		case EToken::NotEqual:	SetInt(!lv || !rv); break;
		case EToken::Plus:
			if(lhs == STR) { SetStr(format("{}{}", *lhs._u._s, rhs.ToStr())); break; }
		default: throw 'n'; //TODO
		}
	}
	else if(lhs == _TRUE_ || rhs == _TRUE_)
	{//todo refactor
		int lv = lhs == _TRUE_ ? 1 : ((lhs == INT && lhs._u._i) || (lhs == FLOAT && lhs._u._f));
		int rv = rhs == _TRUE_ ? 1 : ((rhs == INT && rhs._u._i) || (rhs == FLOAT && rhs._u._f));

		switch(calcOp)
		{
		case EToken::Equal:		SetInt(lv && rv); break;
		case EToken::NotEqual:	SetInt(!lv || !rv); break;
		default: throw 'n'; //TODO
		}
	}
	else if(lhs == _FALSE_ || rhs == _FALSE_)
	{//todo refactor
		int lv = lhs == _FALSE_ ? 1 : ((lhs == INT && !lhs._u._i) || (lhs == FLOAT && !lhs._u._f));
		int rv = rhs == _FALSE_ ? 1 : ((rhs == INT && !rhs._u._i) || (rhs == FLOAT && !rhs._u._f));

		switch(calcOp)
		{
		case EToken::Equal:		SetInt(lv && rv); break;
		case EToken::NotEqual:	SetInt(!lv || !rv); break;
		default: throw 'n'; //TODO
		}
		_type = INT;
	}
	else if(lhs._type == STR || rhs._type == STR)
	{//todo refactor
		switch(calcOp)
		{
		case EToken::Plus: SetStr(format("{}{}", lhs.ToStr(), rhs.ToStr())); break;
		case EToken::Equal:
		case EToken::NotEqual:
			if(lhs == STR && rhs == STR)
			{
				switch(calcOp)
				{
				case EToken::Equal:		SetInt(*lhs._u._s == *rhs._u._s); break;
				case EToken::NotEqual:	SetInt(*lhs._u._s != *rhs._u._s); break;
				}
			}
			else
			{
				SetInt(0);
			}
			break;
		case EToken::Dot:
			if(rhs != STR)
			{//TODO
				throw 'n';
				return false;
			}
			SetAttr(lhs, *rhs._u._s);
			break;
		default:
			//TODO impl
			throw 'n';
			return false;
		}
	}
	else if(lhs._type == FLOAT || rhs._type == FLOAT)
	{
		double lfloat = lhs._type == FLOAT ? lhs._u._f : (double)lhs._u._i;
		double rfloat = rhs._type == FLOAT ? rhs._u._f : (double)rhs._u._i;

		if(calcOp == EToken::Slash && rfloat == 0.0)
		{//TODO div 0
			throw 'n';
		}

		switch(calcOp)
		{
		case EToken::Plus:	SetFloat(lfloat + rfloat); break;
		case EToken::Minus:	SetFloat(lfloat - rfloat); break;
		case EToken::Star:	SetFloat(lfloat * rfloat); break;
		case EToken::Slash:	SetFloat(lfloat / rfloat); break;
		default:
			switch(calcOp)
			{
			case EToken::And:			SetInt(lfloat && rfloat); break;
			case EToken::Or:			SetInt(lfloat || rfloat); break;
			case EToken::Greater:		SetInt(abs(lfloat - rfloat) >= EPSILON && lfloat > rfloat); break;
			case EToken::Less:			SetInt(abs(lfloat - rfloat) >= EPSILON && lfloat < rfloat); break;
			case EToken::GreaterEqual:	SetInt(abs(lfloat - rfloat) < EPSILON || (abs(lfloat - rfloat) >= EPSILON && lfloat > rfloat)); break;
			case EToken::LessEqual:		SetInt(abs(lfloat - rfloat) < EPSILON || (abs(lfloat - rfloat) >= EPSILON && lfloat < rfloat)); break;
			case EToken::Equal:			SetInt(abs(lfloat - rfloat) < EPSILON); break;
			case EToken::NotEqual:		SetInt(abs(lfloat - rfloat) >= EPSILON); break;
			default:
				throw 'n';
			}
		}
	}
	else if(lhs._type == INT && rhs._type == INT)
	{
		int64_t leftInt = lhs._u._i;
		int64_t rightInt = rhs._u._i;

		if(calcOp == EToken::Slash && rightInt == 0)
		{//TODO div 0
			throw 'n';
		}

		switch(calcOp)
		{
		case EToken::Plus:			SetInt(leftInt + rightInt); break;
		case EToken::Minus:			SetInt(leftInt - rightInt); break;
		case EToken::Star:			SetInt(leftInt * rightInt); break;
		case EToken::Slash:			SetInt(leftInt / rightInt); break;
		case EToken::Percent:		SetInt(leftInt % rightInt); break;
		case EToken::And:			SetInt((leftInt != 0) && (rightInt != 0)); break;
		case EToken::Or:			SetInt((leftInt != 0) || (rightInt != 0)); break;
		case EToken::Greater:		SetInt(leftInt > rightInt); break;
		case EToken::Less:			SetInt(leftInt < rightInt); break;
		case EToken::GreaterEqual:	SetInt(leftInt >= rightInt); break;
		case EToken::LessEqual:		SetInt(leftInt <= rightInt); break;
		case EToken::Equal:			SetInt(leftInt == rightInt); break;
		case EToken::NotEqual:		SetInt(leftInt != rightInt); break;
		case EToken::Amp:			SetInt(leftInt & rightInt); break;
		case EToken::Pipe:			SetInt(leftInt | rightInt); break;
		case EToken::Caret:			SetInt(leftInt ^ rightInt); break;
		case EToken::LShift:		SetInt(leftInt << rightInt); break;
		case EToken::RShift:		SetInt(leftInt >> rightInt); break;
		default:
			throw 'n';
		}
	}
	else
	{//TODO
		throw 'n';
		return false;
	}

	return true;
}

bool Variable2::CalcUnaryAndAssign(EToken unaryOp, Variable2& rhs)
{
	if(_type == LVREF)
	{
		auto t = _u._ref;
		if(!t->CalcUnaryAndAssign(unaryOp, rhs))
		{
			throw 'n';
		}
		SetVar(*t);
		return true;
	}

	if(rhs._type == STR)
	{//TODO
		throw 'n';
		return false;
	}

	switch(rhs._type)
	{
	case INT:
		switch(unaryOp)
		{
		case EToken::UnaryPlus: SetInt(+rhs._u._i); break;
		case EToken::UnaryMinus: SetInt(-rhs._u._i); break;
		case EToken::Not: SetInt((int64_t)(!rhs._u._i)); break;
		case EToken::Tilde: SetInt(~rhs._u._i); break;
		default:
			throw 'n';
		}
		return true;
	case FLOAT:
		switch(unaryOp)
		{
		case EToken::UnaryPlus: SetFloat(+rhs._u._f); break;
		case EToken::UnaryMinus: SetFloat(-rhs._u._f); break;
		case EToken::Not: SetFloat(!rhs._u._f); break;
		case EToken::Tilde: SetFloat(0); throw 'n'; break; //TODO
		default:
			throw 'n';
		}
		return true;

	case _NULL_:
		switch(unaryOp)
		{
		case EToken::Not: SetInt(1); break;
		default:
			throw 'n';
		}
		return true;

	case _TRUE_:
		switch(unaryOp)
		{
		case EToken::Not: SetInt(0); break;
		default:
			throw 'n';
		}
		return true;


	case _FALSE_:
		switch(unaryOp)
		{
		case EToken::Not: SetInt(1); break;
		default:
			throw 'n';
		}
		return true;
	}

	throw 'n';
	return false;
}

bool Variable2::CalcIncDec(EToken op)
{
	if(_type == LVREF)
	{
		auto t = _u._ref;
		if(!t->CalcIncDec(op))
		{
			throw 'n';
		}
		SetVar(*t);
		return true;
	}

	switch(op)
	{
	case EToken::PreInc:
	case EToken::PostInc:
		switch(_type)
		{
		case INT: _u._i++; break;
		case FLOAT: _u._f++; break;
		default: throw 'n';
		}
		break;
	case EToken::PreDec:
	case EToken::PostDec:
		switch(_type)
		{
		case INT: _u._i--; break;
		case FLOAT: _u._f--; break;
		default: throw 'n';
		}
		break;
	}
	return true;
}


string Variable2::ToStr() const
{
	switch(_type)
	{
	case NONE: return "none";

	case INT:
		return to_string(_u._i);
	case FLOAT:
		return to_string(_u._f);
	case STR:
		return *_u._s;
	case LIST:
		{
			string r = "[";
			if(!_u._o->_list.empty())
			{
				for(auto& v: _u._o->_list)
				{
					string t = v.ToStr();
					if(v == STR) t = "'" + t + "'";
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
			if(!_u._o->_dict.empty())
			{
				for(auto& [k, v]: _u._o->_dict)
				{
					string t = v.ToStr();
					if(v == STR) t = "'" + t + "'";
					r += format("'{}': {}, ", k, t);
				}
				r.pop_back();
				r.pop_back();
			}
			r += "}";
			return r;
		}
	case LVREF:
	case REF:
		return "ref: " + _u._ref->ToStr();
	case ATTR:
		return "attr(wip): " + _u._attr->name;
	case CLASS:
		return "cls(wip): " + _u._o->_clso._cls->name;
	case _NULL_:
		return "null";
	case _TRUE_:
		return "true";
	case _FALSE_:
		return "false";
	}
	return "";
}


bool Variable2::operator==(Type cmp) const { return _type == cmp; }
bool Variable2::operator!=(Type cmp) const { return _type != cmp; }

const Variable2& Variable2::operator=(const Variable2& rhs)
{
	SetVar(*(Variable2*)&rhs);
	return *this;
}

void Variable2::ResetNewRef()
{
	Clear();
	_u._o = new Object;
	_type = REF;
}
int64_t Variable2::int_() const
{
	if(_type != INT) throw 'n'; //TODO
	return _u._i;
}
double Variable2::float_() const
{
	if(_type != FLOAT) throw 'n'; //TODO
	return _u._f;
}
const std::string& Variable2::str() const
{
	if(_type != STR) throw 'n'; //TODO
	return *_u._s;
}
const Variable2& Variable2::ref() const
{
	if(_type != REF || _type != LVREF) throw 'n'; //TODO
	return *_u._ref;
}
Variable2& Variable2::ref()
{
	if(_type != REF || _type != LVREF) throw 'n'; //TODO
	return *_u._ref;
}
const Attribute2& Variable2::attr() const
{
	if(_type != ATTR) throw 'n'; //TODO
	return *_u._attr;
}
Attribute2& Variable2::attr()
{
	if(_type != ATTR) throw 'n'; //TODO
	return *_u._attr;
}
const ModuleDesc& Variable2::mod() const
{
	if(_type != MODULE) throw 'n'; //TODO
	//qazreturn *_u._mod;
	return *modObj()._mod.modDesc;
}
vector<Variable2>& Variable2::list()
{
	if(_type != LIST) throw 'n'; //TODO
	return _u._o->_list;
}
unordered_map<string, Variable2>& Variable2::dict()
{
	if(_type != DICT) throw 'n'; //TODO
	return _u._o->_dict;
}
const ClassObject2& Variable2::clsObj() const
{
	if(_type != CLASS) throw 'n'; //TODO
	return _u._o->_clso;
}
ClassObject2& Variable2::clsObj()
{
	if(_type != CLASS) throw 'n'; //TODO
	return _u._o->_clso;
}
const ModuleObject& Variable2::modObj() const
{
	//qaz
	if(_type != MODULE && _type != MODULEOBJ) throw 'n'; //TODO
	return _u._o->_modo;
}
ModuleObject& Variable2::modObj()
{
	//qaz
	if(_type != MODULE && _type != MODULEOBJ) throw 'n'; //TODO
	return _u._o->_modo;
}

Variable2::Object::Object()
{
	_refCnt = 1;
}

Variable2::Object::~Object()
{//TODO
	ReleaseRef();
}

void Variable2::Object::AddRef()
{
	_refCnt++;
}

void Variable2::Object::ReleaseRef()
{
	_refCnt--;
	if(_refCnt == 0)
	{
		delete this;
	}
}

}
