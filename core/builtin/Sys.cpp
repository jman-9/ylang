#include "Sys.h"
#include "ylangDefs.h"
#include "util/Args.h"
#include "vm/Variable.h"
#include "vm/RuntimeError.h"
#include "module/ModuleUtil.h"
#include <stdlib.h>
#include <format>
#include <filesystem>


extern char **environ;


namespace ybuiltin::Sys
{
using namespace std;
using namespace yvm;
using namespace ymod;


Module Init()
{//TODO memory leak
	Module o(&GetModuleDesc());
	Variable* v = new Variable;
	v->SetStr(format("ylang {}", YLANG_VER));
	o.memberVars["version"] = YArg{v, YEArg::YVar};

	auto argv = new Variable;
	argv->SetList();
	for(size_t i=1; i<g_Args.size(); i++)
	{
		argv->list().push_back({});
		argv->list().back().SetStr(g_Args[i]);
	}
	o.memberVars["args"] = YArg{argv, YEArg::YVar};

	v = new Variable;
	v->SetStr(g_Args[0]);
	o.memberVars["executable"] = YArg{v, YEArg::YVar};
	return o;
}


YRet Env(YArgs* args)
{
	char **env = environ;

	auto rv = (Variable*)args->retBuff.o;
	rv->SetDict();
	for(; *env; env++)
	{
		string s = *env;
		size_t delimPos = s.find('=');
		if(delimPos == s.npos)
		{
			INTERNALERR("env handling error");
		}

		auto inserted = rv->dict().insert({ s.substr(0, delimPos), {}});
		inserted.first->second.SetStr(s.substr(delimPos+1));
	}
	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

YRet GetEnv(YArgs* args)
{
	MODARG_VAR(0, name, Variable::STR);

	auto rv = (Variable*)args->retBuff.o;

	char* s = getenv(name.str().data());
	if(s == nullptr)
	{
		rv->Clear();
		rv->_type = yvm::Variable::_NULL_;
	}
	else
	{
		rv->SetStr(s);
	}

	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

YRet SetEnv(YArgs* args)
{
	MODARG_VAR(0, n, Variable::STR);
	MODARG_VAR(1, v, Variable::STR);

#ifdef WIN32
	int e = _putenv_s(n.str().data(), v.str().data());
#else
	int e = setenv(n.str().data(), v.str().data(), 1);
#endif

	auto rv = (Variable*)args->retBuff.o;
	rv->SetBool(!e);
	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

const ModuleDesc& GetModuleDesc()
{
	static ModuleDesc m;

	if(m.name.empty())
	{
		m.name = "sys";
		m.builtin = true;
		m.initer = Init;
		m.memberTbl["version"] = ModuleMemberDesc{ "version", ModuleMemberDesc::VAR, };
		m.memberTbl["env"] = ModuleMemberDesc{ "env", ModuleMemberDesc::FUNC, false, 0, Env };
		m.memberTbl["getenv"] = ModuleMemberDesc{ "getenv", ModuleMemberDesc::FUNC, false, 1, GetEnv };
		m.memberTbl["setenv"] = ModuleMemberDesc{ "setenv", ModuleMemberDesc::FUNC, false, 2, SetEnv };
	}
	return m;
}

}
