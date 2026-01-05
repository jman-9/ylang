#include "Variable.h"
#include "RuntimeError.h"
#include <filesystem>


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

	case LVREF:
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

bool Variable::IsBool() const
{
	return _type == _TRUE_ || _type == _FALSE_;
}

bool Variable::IsObject() const
{
	return _type == OBJ || _type == CLASSOBJ || _type == MODULEOBJ || _type == PROGRAMOBJ;
}

void Variable::SetVarLVRef(Variable& lvref, Variable owner)
{
	ResetNewObj();
	_u._o->_lvro._owner.push_back(owner);
	_u._o->_lvro._lvref = &lvref;
	_u._o->_type = LVREF;
	_type = LVREF;
}
void Variable::SetVarLVRef(Variable& lvref)
{
	ResetNewObj();
	_u._o->_lvro._lvref = &lvref;
	_u._o->_type = LVREF;
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
void Variable::SetClass(const Class& cls, bool makeInstance, Variable* prgObj /* = nullptr */)
{
	if(makeInstance)
	{
		if(prgObj == nullptr)
		{
			INTERNALERR(format("{}: must have program object to make an instance", cls.name));
		}

		ResetNewObj();

		_u._o->_clso._cls = &cls;
		_u._o->_clso._prgObjP.push_back(*prgObj);
		_u._o->_clso._prgObj = &(_u._o->_clso._prgObjP[0]);
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
		//except
	}*/
}
void Variable::SetProgram(const Program& prg, bool makeInstance)
{
	if(makeInstance)
	{
		ResetNewObj();

		_u._o->_prgo._prg = &prg;
		_u._o->_prgo._globals;
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
void Variable::SetNull() { Clear(); _type = _NULL_; }
void Variable::SetTrue() { Clear(); _type = _TRUE_; }
void Variable::SetFalse(){ Clear(); _type = _FALSE_; }
void Variable::SetBool(bool b) { b ? SetTrue() : SetFalse(); }

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
	case ATTR: SetAttr(var._u._attr->owner, var._u._attr->name); break;
	case CLASS: SetClass(*var._u._cls, false); break;
	case PROGRAM: SetProgram(*var._u._prg, false); break;

	case LVREF:
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
		INTERNALERR(format("'{}','{}': unsupported for '{}'", TypeStr(), rval.TypeStr(), Token::TokenString(op)));
	}

	if(_type == LVREF)
	{
		auto& lv = lvref();
		if(!lv.Assign(op, rval))
		{
			return false;
		}
		Variable t = lv;
		SetVar(t);
		return true;
	}

	if(op == EToken::Assign)
	{
		SetVar(rval);
	}
	else
	{
		if(_type == LIST || rval._type == LIST)
		{
			if(_type != LIST || rval._type != LIST)
			{
				throw RuntimeError::UnsupportedOperands(op, _type, "", rval._type, "");
				//qaz //TODO
			}
		}
		else if(_type == STR || rval._type == STR)
		{
			if(op == EToken::PlusAssign)
			{
				SetStr(format("{}{}", ToStr(), *rval._u._s));
			}
			else
			{//TODO impl *=, -=, /=
				throw RuntimeError::UnsupportedOperands(op, _type, "", rval._type, "");
				//qaz //TODO
			}
		}
		else if(_type == FLOAT || rval._type == FLOAT)
		{
			if(_type == INT)
			{
				SetFloat((double)_u._i);
			}

			if(_type != FLOAT)
			{
				throw RuntimeError::UnsupportedOperands(op, _type, "", rval._type, "");
				//qaz //TODO
			}

			if(rval._type != FLOAT && rval._type != INT)
			{
				throw RuntimeError::UnsupportedOperands(op, _type, "", rval._type, "");
				//qaz //TODO
			}

			double rfloat = rval._type == FLOAT ? rval._u._f : (double)rval._u._i;

			switch(op)
			{
			case EToken::PlusAssign:	_u._f += rfloat; break;
			case EToken::MinusAssign:	_u._f -= rfloat; break;
			case EToken::MulAssign:		_u._f *= rfloat; break;
			case EToken::DivAssign:		_u._f /= rfloat; break;
			default:
				throw RuntimeError::UnsupportedOperands(op, _type, "", rval._type, "");
				//qaz //TODO
			}
		}
		else if(_type == INT && rval._type == INT)
		{
			if((op == EToken::DivAssign || op == EToken::ModAssign) && rval.int_() == 0)
			{//TODO div 0
				throw RuntimeError::DivideByZero();
				//qaz		 //TODO
			}

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
		{
			throw RuntimeError::UnsupportedOperands(op, _type, "", rval._type, "");
			//qaz //TODO
		}
	}

	return true;
}

bool Variable::CalcAndAssign(Variable& lhs, EToken calcOp, Variable& rhs)
{
	//TODO +, - confusion if(Token::IsPrefixUnary(calcOp))

	if(_type == LVREF)
	{
		auto& lv = lvref();
		if(!lv.CalcAndAssign(lhs, calcOp, rhs))
		{
			return false;
		}
		Variable t = lv;
		SetVar(t);
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
			if(lhs == STR || rhs == STR) { SetStr(format("{}{}", lhs.ToStr(), rhs.ToStr())); break; }
		default:
			throw RuntimeError::UnsupportedOperands(calcOp, lhs._type, "", rhs._type, "");
			//qaz //TODO
		}
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
				throw RuntimeError::UnsupportedOperands(calcOp, lhs._type, "", rhs._type, "");
				//qazSetInt(0);
			}
			break;
		case EToken::Dot:
			if(rhs != STR)
			{
				throw RuntimeError::UnsupportedOperands(calcOp, lhs._type, "", rhs._type, "");
				//qaz			 //TODO
			}
			SetAttr(lhs, *rhs._u._s);
			break;
		default:
			throw RuntimeError::UnsupportedOperands(calcOp, lhs._type, "", rhs._type, "");
			//qaz			//TODO
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
		default:
			throw RuntimeError::UnsupportedOperands(calcOp, lhs._type, "", rhs._type, "");
			//qaz			//TODO
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
		default:
			throw RuntimeError::UnsupportedOperands(calcOp, lhs._type, "", rhs._type, "");
			//qaz			//TODO
		}
		_type = INT;
	}
	else if(lhs._type == FLOAT || rhs._type == FLOAT)
	{
		double lfloat = lhs._type == FLOAT ? lhs._u._f : (double)lhs._u._i;
		double rfloat = rhs._type == FLOAT ? rhs._u._f : (double)rhs._u._i;

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
				throw RuntimeError::UnsupportedOperands(calcOp, lhs._type, "", rhs._type, "");
				//qaz			//TODO
			}
		}
	}
	else if(lhs._type == INT && rhs._type == INT)
	{
		int64_t leftInt = lhs._u._i;
		int64_t rightInt = rhs._u._i;

		if((calcOp == EToken::Slash || calcOp == EToken::Percent) && rightInt == 0)
		{//TODO div 0
			throw RuntimeError::DivideByZero();
			//qaz			//TODO
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
			throw RuntimeError::UnsupportedOperands(calcOp, lhs._type, "", rhs._type, "");
			//qaz			//TODO
		}
	}
	else if(lhs.IsObject() && rhs.IsObject())
	{
		switch(calcOp)
		{
		case EToken::Equal:		SetInt(lhs._u._o == rhs._u._o); break;
		case EToken::NotEqual:	SetInt(lhs._u._o != rhs._u._o); break;
		default:
			throw RuntimeError::UnsupportedOperands(calcOp, lhs._type, "", rhs._type, "");
			//qaz			//TODO
		}
	}
	else
	{//TODO
		throw RuntimeError::UnsupportedOperands(calcOp, lhs._type, "", rhs._type, "");
		//qaz
	}

	return true;
}

bool Variable::CalcUnaryAndAssign(EToken unaryOp, Variable& rhs)
{
	if(_type == LVREF)
	{
		auto& lv = lvref();
		if(!lv.CalcUnaryAndAssign(unaryOp, rhs))
		{//TODO
			return false;
		}
		Variable t = lv;
		SetVar(t);
		return true;
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
			throw RuntimeError::UnsupportedOperand(unaryOp, rhs._type, "");
			//qaz TODO
		}
		return true;
	case FLOAT:
		switch(unaryOp)
		{
		case EToken::UnaryPlus: SetFloat(+rhs._u._f); break;
		case EToken::UnaryMinus: SetFloat(-rhs._u._f); break;
		case EToken::Not: SetFloat(!rhs._u._f); break;
		default:
			throw RuntimeError::UnsupportedOperand(unaryOp, rhs._type, "");
			//qaz TODO
		}
		return true;

	case STR:
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
			throw RuntimeError::UnsupportedOperand(unaryOp, rhs._type, "");
			//qaz TODO
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
			throw RuntimeError::UnsupportedOperand(unaryOp, rhs._type, "");
			//qaz TODO
		}
		return true;
	}

	INTERNALERR(format("'{}': unsupported for '{}'", rhs.TypeStr(), Token::TokenString(unaryOp)));
	return false;
}

bool Variable::CalcIncDec(EToken op)
{
	if(_type == LVREF)
	{
		auto& lv = lvref();
		if(!lv.CalcIncDec(op))
		{//TODO
			return false;
		}
		Variable t = lv;
		SetVar(t);
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
		default:
			throw RuntimeError::UnsupportedOperand(op, _type, "");
			//qaz TODO
		}
		break;
	case EToken::PreDec:
	case EToken::PostDec:
		switch(_type)
		{
		case INT: _u._i--; break;
		case FLOAT: _u._f--; break;
		default:
			throw RuntimeError::UnsupportedOperand(op, _type, "");
			//qaz TODO
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
	case LVREF:
		return "ref: " + lvref().ToStr();
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
	case LVREF:		return lvref().IsNullOrFalse();
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

bool Variable::bool_() const
{
	if(!IsBool()) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return _type == Variable::_TRUE_;
}
int64_t Variable::int_() const
{
	if(_type != INT) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return _u._i;
}
double Variable::float_() const
{
	if(_type != FLOAT) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return _u._f;
}
const std::string& Variable::str() const
{
	if(_type != STR) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return *_u._s;
}
const Variable& Variable::lvref() const
{
	if(_type != LVREF) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return *_u._o->_lvro._lvref;
}
Variable& Variable::lvref()
{
	if(_type != LVREF) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return *_u._o->_lvro._lvref;
}
const Attribute& Variable::attr() const
{
	if(_type != ATTR) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return *_u._attr;
}
Attribute& Variable::attr()
{
	if(_type != ATTR) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return *_u._attr;
}

const Class& Variable::cls() const
{
	if(_type == CLASS)
		return *_u._cls;
	else if(_type == CLASSOBJ)
		return *_u._o->_clso._cls;
	else
		INTERNALERR(format("{}: incorrect type", TypeStr()));
}

const ModuleDesc& Variable::mod() const
{
	if(_type != MODULE) INTERNALERR(format("{}: incorrect type", TypeStr()));
	//qaz TODO return *_u._mod;
	return *modObj()._mod.modDesc;
}

const Program& Variable::prg() const
{
	if(_type == PROGRAM)
		return *_u._prg;
	else if(_type == PROGRAMOBJ)
		return *_u._o->_prgo._prg;
	else
		INTERNALERR(format("{}: incorrect type", TypeStr()));
}

const std::vector<Variable>& Variable::list() const
{
	if(_type != LIST) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return _u._o->_list;
}
vector<Variable>& Variable::list()
{
	if(_type != LIST) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return _u._o->_list;
}
const std::unordered_map<std::string, Variable>& Variable::dict() const
{
	if(_type != DICT) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return _u._o->_dict;
}
unordered_map<string, Variable>& Variable::dict()
{
	if(_type != DICT) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return _u._o->_dict;
}
const ClassObject& Variable::clsObj() const
{
	if(_type != CLASSOBJ) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return _u._o->_clso;
}
ClassObject& Variable::clsObj()
{
	if(_type != CLASSOBJ) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return _u._o->_clso;
}
const ModuleObject& Variable::modObj() const
{
	//qaz TODO arrange
	if(_type != MODULE && _type != MODULEOBJ) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return _u._o->_modo;
}
ModuleObject& Variable::modObj()
{
	//qaz TODO arrange
	if(_type != MODULE && _type != MODULEOBJ) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return _u._o->_modo;
}
const ProgramObject& Variable::prgObj() const
{
	if(_type != PROGRAMOBJ) INTERNALERR(format("{}: incorrect type", TypeStr()));
	return _u._o->_prgo;
}
ProgramObject& Variable::prgObj()
{
	if(_type != PROGRAMOBJ) INTERNALERR(format("{}: incorrect type", TypeStr()));
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
	default:
		INTERNALERR(format("{}: incorrect type", (int)o.tp));
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
	default:
		INTERNALERR(format("{}: unsupported type", TypeStr()));
	}
	return YArg();
}

string_view Variable::TypeStr(Type t)
{
	switch(t)
	{
	case NONE: return "none";
	case INT: return "int";
	case FLOAT: return "float";
	case STR: return "string";
	case LVREF: return "lvalue_reference";
	case ATTR: return "attribute";
	case CLASS: return "class";
	case MODULE: return "module";
	case PROGRAM: return "program";
	case OBJ: return "object";
	case LIST: return "list";
	case DICT: return "dict";
	case CLASSOBJ: return "class_instance";
	case MODULEOBJ: return "module_instance";
	case PROGRAMOBJ: return "program_instance";
	case _NULL_: return "null";
	case _TRUE_: return "boolean_true";
	case _FALSE_: return "boolean_false";
	}
	return "";
}

string_view Variable::TypeStr() const
{
	return TypeStr(_type);
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
