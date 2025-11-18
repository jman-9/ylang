#include "Math.h"
#include "vm/Variable.h"
#include <math.h>


namespace ybuiltin::Math
{
using namespace yvm;

YRet Sin(YArgs* args)
{
	auto a1 = (Variable*)args->args[0].o;
	double x = *a1 == Variable::INT ? (double)a1->_int : a1->_float;
	double v = sin(x);

	YRet yr;
	yr.single.o = Variable::NewFloat(v);
	yr.single.tp = YEArg::YVar;
	return yr;
}

YRet Cos(YArgs* args)
{
	auto a1 = (Variable*)args->args[0].o;
	double x = *a1 == Variable::INT ? (double)a1->_int : a1->_float;
	double v = cos(x);

	YRet yr;
	yr.single.o = Variable::NewFloat(v);
	yr.single.tp = YEArg::YVar;
	return yr;
}

YRet Tan(YArgs* args)
{
	auto a1 = (Variable*)args->args[0].o;
	double x = *a1 == Variable::INT ? (double)a1->_int : a1->_float;
	double v = tan(x);

	YRet yr;
	yr.single.o = Variable::NewFloat(v);
	yr.single.tp = YEArg::YVar;
	return yr;
}

YRet Sqrt(YArgs* args)
{
	auto a1 = (Variable*)args->args[0].o;
	double x = *a1 == Variable::INT ? (double)a1->_int : a1->_float;
	double v = sqrt(x);

	YRet yr;
	yr.single.o = Variable::NewFloat(v);
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
