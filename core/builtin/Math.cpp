#include "Math.h"
#include "vm/Variable2.h"
#include <math.h>


namespace ybuiltin::Math
{
using namespace yvm;

YRet Container(YArgs* args, double(*func)(double))
{
	auto a1 = (Variable2*)args->args[0].o;
	double x = *a1 == Variable2::INT ? (double)a1->int_() : a1->float_();
	double v = func(x);

	YRet yr;
	auto rv = (Variable2*)args->retBuff.o;
	rv->SetFloat(v);
	yr.single.o = rv;
	yr.single.tp = YEArg::YVar;
	return yr;
}

YRet Sin(YArgs* args)	{ return Container(args, sin); }
YRet Cos(YArgs* args)	{ return Container(args, cos); }
YRet Tan(YArgs* args)	{ return Container(args, tan); }
YRet Sqrt(YArgs* args)	{ return Container(args, sqrt); }
YRet Floor(YArgs* args)	{ return Container(args, floor); }

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
		m.memberTbl[ "floor" ] = { "floor", ymod::ModuleMemberDesc::FUNC, false, 1, Floor};
	}
	return m;
}

}
