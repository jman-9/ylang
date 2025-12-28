#include "Math.h"
#include "vm/Variable.h"
#include "module/ModuleUtil.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>


namespace ybuiltin::Math
{
using namespace yvm;
using namespace ymod;
using namespace std;


Module Init()
{
	Module o(&GetModuleDesc());
	Variable* v = new Variable;
	v->SetFloat(M_PI);
	o.memberVars["pi"] = YArg{v, YEArg::YVar};
	return o;
}


YRet Container1(YArgs* args, double(*func)(double))
{
	MODARG_VAR2(0, a1, Variable::INT, Variable::FLOAT);
	double x = a1 == Variable::INT ? (double)a1.int_() : a1.float_();
	double v = func(x);

	YRet yr;
	auto rv = (Variable*)args->retBuff.o;
	rv->SetFloat(v);
	yr.single.o = rv;
	yr.single.tp = YEArg::YVar;
	return yr;
}

YRet Container2(YArgs* args, double(*func)(double, double))
{
	MODARG_VAR2(0, a1, Variable::INT, Variable::FLOAT);
	MODARG_VAR2(1, a2, Variable::INT, Variable::FLOAT);
	double x = a1 == Variable::INT ? (double)a1.int_() : a1.float_();
	double y = a2 == Variable::INT ? (double)a2.int_() : a2.float_();
	double v = func(x, y);

	YRet yr;
	auto rv = (Variable*)args->retBuff.o;
	rv->SetFloat(v);
	yr.single.o = rv;
	yr.single.tp = YEArg::YVar;
	return yr;
}

YRet Container3(YArgs* args, double( *funcf)(double), int64_t(*funci)(int64_t))
{
	MODARG_VAR2(0, a1, Variable::INT, Variable::FLOAT);

	auto rv = (Variable*)args->retBuff.o;
	if(a1 == Variable::FLOAT)
	{
		rv->SetFloat(funcf(a1.float_()));
	}
	else
	{
		rv->SetInt(funci(a1.int_()));
	}

	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

YRet Container4(YArgs* args, const double&( *funcf)(const double&, const double&), const int64_t&(*funci)(const int64_t&, const int64_t&))
{
	MODARG_VAR2(0, a1, Variable::INT, Variable::FLOAT);
	MODARG_VAR2(1, a2, Variable::INT, Variable::FLOAT);

	auto rv = (Variable*)args->retBuff.o;
	if(a1 == Variable::FLOAT || a2 == Variable::FLOAT)
	{
		double x = a1 == Variable::INT ? (double)a1.int_() : a1.float_();
		double y = a2 == Variable::INT ? (double)a2.int_() : a2.float_();
		rv->SetFloat(funcf(x, y));
	}
	else
	{
		rv->SetInt(funci(a1.int_(), a2.int_()));
	}

	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

YRet Sin(YArgs* args)	{ return Container1(args, sin); }
YRet Cos(YArgs* args)	{ return Container1(args, cos); }
YRet Tan(YArgs* args)	{ return Container1(args, tan); }
YRet Sqrt(YArgs* args)	{ return Container1(args, sqrt); }
YRet Floor(YArgs* args)	{ return Container1(args, floor); }
YRet Ceil(YArgs* args)	{ return Container1(args, ceil); }
YRet Round(YArgs* args)	{ return Container1(args, round); }
YRet Pow(YArgs* args)	{ return Container2(args, pow); }

double yabsd(double v) { return v >= 0.0 ? v : -v; }
int64_t yabsi(int64_t v) { return v >= 0 ? v : -v; }
YRet Abs(YArgs* args) { return Container3(args, yabsd, yabsi); }

YRet Min(YArgs* args) { return Container4(args, min<double>, min<int64_t>); }
YRet Max(YArgs* args) { return Container4(args, max<double>, max<int64_t>); }


const ymod::ModuleDesc& GetModuleDesc()
{
	static ymod::ModuleDesc m;

	if(m.name.empty())
	{
		m.name = "math";
		m.initer = Init;
		m.builtin = true;
		m.memberTbl[ "sin" ] = { "sin", ymod::ModuleMemberDesc::FUNC, false, 1, Sin};
		m.memberTbl[ "cos" ] = { "cos", ymod::ModuleMemberDesc::FUNC, false, 1, Cos};
		m.memberTbl[ "tan" ] = { "tan", ymod::ModuleMemberDesc::FUNC, false, 1, Tan};
		m.memberTbl[ "sqrt" ] = { "sqrt", ymod::ModuleMemberDesc::FUNC, false, 1, Sqrt};
		m.memberTbl[ "floor" ] = { "floor", ymod::ModuleMemberDesc::FUNC, false, 1, Floor};
		m.memberTbl[ "ceil" ] = { "ceil", ymod::ModuleMemberDesc::FUNC, false, 1, Ceil};
		m.memberTbl[ "round" ] = { "round", ymod::ModuleMemberDesc::FUNC, false, 1, Round};
		m.memberTbl[ "pow" ] = { "pow", ymod::ModuleMemberDesc::FUNC, false, 2, Pow};
		m.memberTbl[ "abs" ] = { "abs", ymod::ModuleMemberDesc::FUNC, false, 1, Abs};
		m.memberTbl[ "min" ] = { "min", ymod::ModuleMemberDesc::FUNC, false, 2, Min};
		m.memberTbl[ "max" ] = { "max", ymod::ModuleMemberDesc::FUNC, false, 2, Max};

	}
	return m;
}

}
