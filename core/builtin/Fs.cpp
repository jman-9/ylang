#include "Fs.h"
#include "Path.h"
#include "vm/Variable.h"
#include "vm/RuntimeError.h"
#include "module/ModuleUtil.h"
#include <stdlib.h>
#include <filesystem>


namespace ybuiltin::Fs
{
using namespace yvm;
using namespace ymod;
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

YRet AbsPath(YArgs* args)
{
	MODARG_VAR(0, path, Variable::STR);

	auto rv = (Variable*)args->retBuff.o;
	rv->SetStr(filesystem::absolute(path.str()).string());
	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}


Module Init()
{//TODO memory leak
	Module o(&GetModuleDesc());
	Variable* v = new Variable;
	v->SetModule(Path::GetModuleDesc(), false);
	o.memberVars["path"] = YArg{v, YEArg::YVar};
	return o;
}

const ModuleDesc& GetModuleDesc()
{
	static ymod::ModuleDesc m;

	if(m.name.empty())
	{
		m.name = "fs";
		m.initer = Init;
		m.builtin = true;
		m.memberTbl[ "exists" ] = { "exists", ymod::ModuleMemberDesc::FUNC, false, 1, Exists };
		m.memberTbl[ "cwd" ] = { "cwd", ymod::ModuleMemberDesc::FUNC, false, 0, Cwd };
		m.memberTbl[ "abspath" ] = { "abs", ymod::ModuleMemberDesc::FUNC, false, 1, AbsPath };
	}
	return m;
}

}
