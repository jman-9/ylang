#include "Variable.h"


namespace yvm
{
using namespace std;
using namespace ymod;

#define EPSILON (1e-9)


Variable::Variable()
{
	_type = NONE;
	_u._s = nullptr;
}
Variable::~Variable()
{
	Clear();
}

Variable::Variable(const Variable& v)
	: Variable()
{
	SetVar(*(Variable*)&v);
}
const Variable& Variable::operator=(const Variable& rhs)
{
	SetVar(*(Variable*)&rhs);
	return *this;
}
void Variable::Clear()
{
	switch(_type)
	{
	case STR:
		if(_u._s) { delete _u._s; _u._s = nullptr; }
		break;

	case ATTR:
		if(_u._attr) { delete _u._attr; _u._attr = nullptr; }
		break;

	case OBJ:
	case LIST:
	case DICT:
	case CLASS:
	case CLASSOBJ:
	case MODULEOBJ:
	case PROGRAMOBJ:
		if(_u._o) { _u._o->ReleaseRef(); _u._o = nullptr; }
		break;
	}

	_type = NONE;
}


void Variable::SetInt(int64_t i)
{
	Clear();
	_u._i = i;
	_type = INT;
}
void Variable::SetFloat(double f)
{
	Clear();
	_u._f = f;
	_type = FLOAT;
}
void Variable::SetStr(string s)
{
	Clear();
	_u._s = new string(s);
	_type = STR;
}
void Variable::SetObj(Object* obj)
{
	auto t = obj;
	obj->AddRef();

	Clear();
	_u._o = obj;
	_u._o->AddRef();
	_type = obj->_type;

	t->ReleaseRef();
}

bool Variable::IsObject() const
{
	return _type == OBJ || _type == CLASSOBJ || _type == MODULEOBJ || _type == PROGRAMOBJ;
}

void Variable::SetVarRef(Variable& var)
{
	Clear();
	_u._ref = &var;
	_type = REF;

}
void Variable::SetVarLVRef(Variable& var)
{
	Clear();
	_u._ref = &var;
	_type = LVREF;
}
void Variable::SetAttr(Variable& owner, string name)
{
	auto t = new Attribute{owner, name};
	Clear();
	_u._attr = t;
	_type = ATTR;
}
void Variable::SetAttr(Attribute& attr)
{
	SetAttr(attr.owner, attr.name);
}
void Variable::SetList(const std::vector<Variable>& list /*= std::vector<Variable>()*/)
{
	ResetNewObj();
	for(auto& e : list)
		_u._o->_list.push_back(e);
	_u._o->_type = LIST;
	_type = LIST;
}
void Variable::SetDict(const std::unordered_map<std::string, Variable>& dict /*= std::unordered_map<std::string, Variable>()*/)
{	ResetNewObj();
	for(auto& e : dict)
		_u._o->_dict.insert(e);
	_u._o->_type = DICT;
	_type = DICT;
}
void Variable::SetClass(const Class& cls, Variable* prg, bool makeInstance)
{
	if(makeInstance)
	{
		ResetNewObj();

		_u._o->_clso._cls = &cls;
		if(prg) _u._o->_clso._prgObj.push_back(*prg);
		_u._o->_type = CLASSOBJ;
		_type = CLASSOBJ;

		clsObj()._fields.resize(clsObj()._cls->_fields.size());
	}
	else
	{
		Clear();
		_u._cls = &cls;
		_type = CLASS;
	}
}
void Variable::SetModule(const ModuleDesc& mod, bool makeInstance)
{
	ResetNewObj();

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
void Variable::SetProgram(const Program& prg, bool makeInstance)
{
	if(makeInstance)
	{
		ResetNewObj();

		_u._o->_prgo._prg = &prg;
		_u._o->_prgo._lsp = 0;
		_u._o->_prgo._local.resize(33);
		_u._o->_type = PROGRAMOBJ;
		_type = PROGRAMOBJ;
	}
	else
	{
		Clear();
		_u._prg = &prg;
		_type = PROGRAM;
	}
}

void Variable::SetVar(Variable& var)
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
	case CLASS: SetClass(*var._u._cls, nullptr, false); break;
	case PROGRAM: SetProgram(*var._u._prg, false); break;

	case OBJ:
	case LIST:
	case DICT:
	case CLASSOBJ:
	case MODULE:
	case MODULEOBJ:
	case PROGRAMOBJ: SetObj(var._u._o); break;

	case _NULL_:
	case _TRUE_:
	case _FALSE_: Clear(); _type = var._type; break;
	}
}

bool Variable::Assign(EToken op, Variable& rval)
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

bool Variable::CalcAndAssign(Variable& lhs, EToken calcOp, Variable& rhs)
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
	else if(lhs.IsObject() && rhs.IsObject())
	{
		switch(calcOp)
		{
		case EToken::Equal:		SetInt(lhs._u._o == rhs._u._o); break;
		case EToken::NotEqual:	SetInt(lhs._u._o != rhs._u._o); break;
		default: throw 'n'; //TODO
		}
	}
	else
	{//TODO
		throw 'n';
		return false;
	}

	return true;
}

bool Variable::CalcUnaryAndAssign(EToken unaryOp, Variable& rhs)
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

	case REF:
	case LVREF:
	case ATTR:
	case CLASS:
	case MODULE:
	case LIST:
	case DICT:
	case CLASSOBJ:
	case MODULEOBJ:
	case _TRUE_:
		switch(unaryOp)
		{
		case EToken::Not: SetInt(0); break;
		default:
			throw 'n';
		}
		return true;

	case NONE:
	case OBJ:
	case _NULL_:
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

bool Variable::CalcIncDec(EToken op)
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


string Variable::ToStr() const
{
	switch(_type)
	{
	case NONE: return "none";

	case INT:
		return to_string(int_());
	case FLOAT:
		return to_string(float_());
	case STR:
		return str();
	case REF:
	case LVREF:
		return "ref: " + ref().ToStr();
	case ATTR:
		return "attr: " + attr().name;
	case CLASS:
		return "class: " + cls().name;
	case MODULE:
		return "module: " + mod().name;
	case PROGRAM:
		return "program: (WIP)";

	case OBJ:
		return "obj: (uninitialized)";
	case LIST:
		{
			string r = "[";
			if(!list().empty())
			{
				for(auto& v: list())
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
			if(!dict().empty())
			{
				for(auto& [k, v]: dict())
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
	case CLASSOBJ:
		return "classobj: " + clsObj()._cls->name;
	case MODULEOBJ:
		return "moduleobj: " + modObj()._mod.modDesc->name;
	case PROGRAMOBJ:
		return "programobj: (WIP)";

	case _NULL_:
		return "null";
	case _TRUE_:
		return "true";
	case _FALSE_:
		return "false";
	}
	return "";
}


bool Variable::IsNullOrFalse() const
{
	switch(_type)
	{
	case NONE:
	case OBJ:
	case _NULL_:
	case _FALSE_:
		return true;

	case INT:		return !int_();
	case FLOAT:		return !float_();
	case STR:		return str().empty();
	case REF:
	case LVREF:		return ref().IsNullOrFalse();
	case ATTR:		return attr().name.empty();
	case CLASS:		return !_u._cls || cls().name.empty();
	case MODULE:	return mod().IsNull(); //TODO qaz !_u._mod ||
	case PROGRAM:	return !_u._prg || prg()._mainCode.empty(); //TODO
	case LIST:		return list().empty();
	case DICT:		return dict().empty();
	case CLASSOBJ:	return !clsObj()._cls;
	case MODULEOBJ:	return !modObj()._mod.modDesc;
	case PROGRAMOBJ:	return !prgObj()._prg;
	}
	return false;
}
bool Variable::operator==(Type cmp) const { return _type == cmp; }
bool Variable::operator!=(Type cmp) const { return _type != cmp; }

void Variable::ResetNewObj()
{
	Clear();
	_u._o = new Object;
	_u._o->_type = OBJ;
	_type = OBJ;
}

int64_t Variable::int_() const
{
	if(_type != INT) throw 'n'; //TODO
	return _u._i;
}
double Variable::float_() const
{
	if(_type != FLOAT) throw 'n'; //TODO
	return _u._f;
}
const std::string& Variable::str() const
{
	if(_type != STR) throw 'n'; //TODO
	return *_u._s;
}
const Variable& Variable::ref() const
{
	if(_type != REF && _type != LVREF) throw 'n'; //TODO
	return *_u._ref;
}
Variable& Variable::ref()
{
	if(_type != REF && _type != LVREF) throw 'n'; //TODO
	return *_u._ref;
}
const Attribute& Variable::attr() const
{
	if(_type != ATTR) throw 'n'; //TODO
	return *_u._attr;
}
Attribute& Variable::attr()
{
	if(_type != ATTR) throw 'n'; //TODO
	return *_u._attr;
}

const Class& Variable::cls() const
{
	if(_type == CLASS)
		return *_u._cls;
	else if(_type == CLASSOBJ)
		return *_u._o->_clso._cls;
	else
		throw 'n'; //TODO
}

const ModuleDesc& Variable::mod() const
{
	if(_type != MODULE) throw 'n'; //TODO
	//qazreturn *_u._mod;
	return *modObj()._mod.modDesc;
}

const Program& Variable::prg() const
{
	if(_type == PROGRAM)
		return *_u._prg;
	else if(_type == PROGRAMOBJ)
		return *_u._o->_prgo._prg;
	else
		throw 'n'; //TODO
}

const std::vector<Variable>& Variable::list() const
{
	if(_type != LIST) throw 'n'; //TODO
	return _u._o->_list;
}
vector<Variable>& Variable::list()
{
	if(_type != LIST) throw 'n'; //TODO
	return _u._o->_list;
}
const std::unordered_map<std::string, Variable>& Variable::dict() const
{
	if(_type != DICT) throw 'n'; //TODO
	return _u._o->_dict;
}
unordered_map<string, Variable>& Variable::dict()
{
	if(_type != DICT) throw 'n'; //TODO
	return _u._o->_dict;
}
const ClassObject& Variable::clsObj() const
{
	if(_type != CLASSOBJ) throw 'n'; //TODO
	return _u._o->_clso;
}
ClassObject& Variable::clsObj()
{
	if(_type != CLASSOBJ) throw 'n'; //TODO
	return _u._o->_clso;
}
const ModuleObject& Variable::modObj() const
{
	//qaz
	if(_type != MODULE && _type != MODULEOBJ) throw 'n'; //TODO
	return _u._o->_modo;
}
ModuleObject& Variable::modObj()
{
	//qaz
	if(_type != MODULE && _type != MODULEOBJ) throw 'n'; //TODO
	return _u._o->_modo;
}
const ProgramObject& Variable::prgObj() const
{
	if(_type != PROGRAMOBJ) throw 'n'; //TODO
	return _u._o->_prgo;
}
ProgramObject& Variable::prgObj()
{
	if(_type != PROGRAMOBJ) throw 'n'; //TODO
	return _u._o->_prgo;
}

void Variable::SetValueFromContract(YArg o)
{
	switch(o.tp)
	{
	case YEArg::Int64: return SetInt(o.ToInt64());
	case YEArg::Double: return SetFloat(o.ToDouble());
	case YEArg::Str: return SetStr(o.ToStr());
	case YEArg::YVar: SetVar(*(Variable*)o.o); break;
	default://TODO qaz
		throw 'n';
	}
}
YArg Variable::ToContract() const
{
	auto ToYStr = [](const string& s) -> YStr*
	{
		YStr* ys = new YStr;
		ys->str = new char[s.size()];
		memcpy(ys->str, s.data(), s.size());
		ys->len = (int)s.size();
		return ys;
	};
	auto ToList = [](const vector<Variable>& list) -> YList*
	{
		YList* yl = new YList;
		yl->sz = (int)list.size();
		yl->list = new YArg[yl->sz];
		for(int i=0; i<yl->sz; i++)
		{
			yl->list[i] = list[i].ToContract();
		}
		return yl;
	};
	auto ToDict = [ToYStr](const unordered_map<string, Variable>& dict) -> YDict*
	{
		YDict* yd = new YDict;
		yd->sz = (int)dict.size();
		yd->keys = new YArg[yd->sz];
		yd->vals = new YArg[yd->sz];
		auto it = dict.begin();
		for(int i=0; i<yd->sz; i++, it++)
		{
			yd->keys[i] = { (void*)ToYStr(it->first), YEArg::Str };
			yd->vals[i] = it->second.ToContract();
		}
		return yd;
	};

	switch(_type)
	{
	case INT: return { (void*)(intptr_t)int_(), YEArg::Int64 };
	case FLOAT: return { (void*)(*(intptr_t*)&_u._f), YEArg::Double };
	case STR: return { (void*)ToYStr(str()), YEArg::Str };
	case LIST: return { (void*)ToList(list()), YEArg::List };
	case DICT: return { (void*)ToDict(dict()), YEArg::Dict };
	case MODULEOBJ: return { modObj()._o, YEArg::Object };
	default: throw 'n';
	}
	return YArg();
}


Variable::Object::Object()
{
	_refCnt = 1;
}

Variable::Object::~Object()
{//TODO
}

void Variable::Object::AddRef()
{
	_refCnt++;
}

void Variable::Object::ReleaseRef()
{
	int r = --_refCnt;
	if(r == 0)
	{
		delete this;
	}
}

}
