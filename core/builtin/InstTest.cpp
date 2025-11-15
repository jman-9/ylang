#include "Math.h"
#include "vm/Variable.h"


namespace ybuiltin::InstTest
{

YRet Issuer(YArgs* args)
{
	YObj* yo = new YObj { .name = YStr(), .obj = (void*)1234 };

	YRet yr;
	yr.single.tp = YEArg::Object;
	yr.single.o = yo;
	return yr;
}

YRet Test(YArgs* args)
{
	YRet yr;
	return yr;
}

const ymod::Module& GetModule()
{
	static ymod::Module m;

	if(m.name.empty())
	{
		m.name = "insttest";
		m.newer = Issuer;
		m.builtin = true;
		m.funcTbl[ "test" ] = { "test", true, 0, Test };
	}
	return m;
}

}
