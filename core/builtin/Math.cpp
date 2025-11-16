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
	yr.single.FromDouble(v);

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
		m.funcTbl[ "sin" ] = { "sin", false, 1, Sin};
	}
	return m;
}

}
