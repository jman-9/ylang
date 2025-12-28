#include "Time.h"
#include "vm/Variable.h"
#include "vm/RuntimeError.h"
#include "module/ModuleUtil.h"
#include <stdlib.h>
#include <filesystem>


namespace ybuiltin::Fs
{
using namespace yvm;
using namespace std;


YRet Exists(YArgs* args)
{
	MODARG_VAR(0, path, Variable::STR);

	auto rv = (Variable*)args->retBuff.o;
	rv->SetBool(filesystem::exists(path.str()));
	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

YRet Cwd(YArgs* args)
{
	auto rv = (Variable*)args->retBuff.o;
	if(args->numArgs > 0)
	{
		MODARG_VAR(0, path, Variable::STR);
		if(!filesystem::exists(path.str()))
		{
			rv->SetBool(false);
		}
		else
		{
			filesystem::current_path(path.str());
			rv->SetBool(true);
		}
	}
	else
	{
		rv->SetStr(filesystem::current_path().string());
	}
	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
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
		m.name = "fs";
		m.builtin = true;
		m.memberTbl[ "exists" ] = { "exists", ymod::ModuleMemberDesc::FUNC, false, 1, Exists };
		m.memberTbl[ "cwd" ] = { "cwd", ymod::ModuleMemberDesc::FUNC, false, 0, Cwd };
		//m.memberTbl[ "" ] = { "cwd", ymod::ModuleMemberDesc::FUNC, false, 0, Cwd };
		//m.memberTbl[ "run" ] = { "run", ymod::ModuleMemberDesc::FUNC, false, 1, Run };
	}
	return m;
}

}
