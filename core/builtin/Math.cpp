#include "Math.h"
#include "vm/Variable.h"


namespace ybuiltin::Math
{

YRet Sin(YArgs* args)
{//TODO int value check
	auto a1 = (yvm::Variable*)args->args[0].o;
	double x = a1->_float;
	double v = sin(x);
	YRet yr;
	yr.single.SetDouble(v);

	auto vr = new yvm::Variable;
	vr->SetFloat(v);
	yr.single.o = (void*)vr;
	yr.single.tp = YEArg::YVar;
	return yr;
}

YRet Cos(YArgs* args)
{//TODO int value check
	auto a1 = (yvm::Variable*)args->args[0].o;
	double x = a1->_float;
	double v = cos(x);
	YRet yr;
	yr.single.SetDouble(v);

	auto vr = new yvm::Variable;
	vr->SetFloat(v);
	yr.single.o = (void*)vr;
	yr.single.tp = YEArg::YVar;
	return yr;
}

YRet Tan(YArgs* args)
{//TODO int value check
	auto a1 = (yvm::Variable*)args->args[0].o;
	double x = a1->_float;
	double v = tan(x);
	YRet yr;
	yr.single.SetDouble(v);

	auto vr = new yvm::Variable;
	vr->SetFloat(v);
	yr.single.o = (void*)vr;
	yr.single.tp = YEArg::YVar;
	return yr;
}

YRet Sqrt(YArgs* args)
{//TODO int value check
	auto a1 = (yvm::Variable*)args->args[0].o;
	double x = a1->_float;
	double v = sqrt(x);
	YRet yr;
	yr.single.SetDouble(v);

	auto vr = new yvm::Variable;
	vr->SetFloat(v);
	yr.single.o = (void*)vr;
	yr.single.tp = YEArg::YVar;
	return yr;
}

const ymod::ModuleDesc& GetModuleDesc()
{
	static ymod::ModuleDesc m;

	if(m.name.empty())
	{
		m.name = "math";
		m.builtin = true;
		m.memberTbl[ "sin" ] = { "sin", ymod::ModuleMemberDesc::FUNC, false, 1, Sin};
		m.memberTbl[ "cos" ] = { "cos", ymod::ModuleMemberDesc::FUNC, false, 1, Cos};
		m.memberTbl[ "tan" ] = { "tan", ymod::ModuleMemberDesc::FUNC, false, 1, Tan};
		m.memberTbl[ "sqrt" ] = { "sqrt", ymod::ModuleMemberDesc::FUNC, false, 1, Sqrt};
	}
	return m;
}

}
