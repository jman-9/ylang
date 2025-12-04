#include "Variable2.h"
using namespace std;


namespace yvm
{

#define EPSILON (1e-9)


Variable2::Variable2()
{
	_type = NONE;
	_u._s = nullptr;
}

void Variable2::Clear()
{
	switch(_type)
	{
	case STR:
		if(_u._s) { delete _u._s; _u._s = nullptr; }
		break;

	case LIST:
	case DICT:
		if(_u._obj) { _u._obj->ReleaseRef(); _u._obj = nullptr; }
		break;

	case ATTR:
		if(_u._attr) { delete _u._attr; _u._attr = nullptr; }
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
void Variable2::SetStr(const string& s)
{
	Clear();
	_u._s = new string(s);
	_type = STR;
}
void Variable2::SetObj(Object* obj)
{
	Clear();
	_u._obj = obj;
	_u._obj->AddRef();
	_type = obj->_type;
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
void Variable2::SetAttr(Attribute& attr)
{
	Clear();
	_u._attr = new Attribute{attr.owner, attr.name};
	_type = ATTR;
}
void Variable2::SetVar(Variable2& var)
{
	switch(var._type)
	{
	case NONE: Clear(); break;
	case INT: SetInt(var._u._i); break;
	case FLOAT: SetFloat(var._u._f); break;
	case STR: SetStr(*var._u._s); break;
	case REF: SetVarRef(var); break;
	case LVREF: SetVarLVRef(var); break;
	case ATTR: SetAttr(*var._u._attr); break;
	case MODULE: break; //TODO

	case LIST:
	case DICT:
	case CLASS:
	case CLASSOBJ: SetObj(var._u._obj); break;
	}
}


void Variable2::ResetNewRef()
{
/*	Clear();
	_u._obj = new Object;
	_type = REF;*/
}


Variable2::Object::Object()
{
	_refCnt = 1;
}

Variable2::Object::~Object()
{//TODO
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
