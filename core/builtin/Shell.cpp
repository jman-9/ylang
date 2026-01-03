#include "Shell.h"
#include "vm/Variable.h"
#include "vm/RuntimeError.h"
#include "module/ModuleUtil.h"
#include <stdlib.h>


namespace ybuiltin::Shell
{
using namespace yvm;
using namespace std;


YRet System(YArgs* args)
{
	MODARG_VAR(0, cmd, Variable::STR);

	auto rv = (Variable*)args->retBuff.o;
	rv->SetInt(system(cmd.str().data()));
	YRet yr;
	yr.single.SetYVar(rv);
	return {};
}

YRet Run(YArgs* args)
{
	MODARG_VAR(0, cmd, Variable::STR);

	auto rv = (Variable*)args->retBuff.o;
#ifdef WIN32
	FILE* fp = _popen(cmd.str().data(), "rt");
	if(!fp)
	{
		rv->SetNull();
	}
#else
	FILE* fp = nullptr;
#endif

	string s;
	char buf[128];
	for( ; fgets(buf, 127, fp); )
	{
		s += buf;
	}

	if(feof(fp))
	{
		rv->SetStr(s);
	}
	else
	{
		rv->SetNull();
	}

	fclose(fp);

	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}


const ymod::ModuleDesc& GetModuleDesc()
{
	static ymod::ModuleDesc m;

	if(m.name.empty())
	{
		m.name = "shell";
		m.builtin = true;
		m.memberTbl[ "system" ] = { "system", ymod::ModuleMemberDesc::FUNC, false, 1, System };
		m.memberTbl[ "run" ] = { "run", ymod::ModuleMemberDesc::FUNC, false, 1, Run };
	}
	return m;
}

}
