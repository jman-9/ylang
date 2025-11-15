#include "Math.h"
#include "vm/Variable.h"


namespace ybuiltin::InstTest
{

YRet Issuer(YArgs* args)
{
	YRet yr;
	yr.single.tp = YEObj::License;
	yr.single.o = (void*)1234;
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
		m.issuer = Issuer;
		m.builtin = true;
		m.funcTbl[ "test" ] = { "test", true, 0, Test };
	}
	return m;
}

}
