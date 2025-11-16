#include "Sys.h"
#include "Args.h"
#include "vm/Variable.h"


namespace ybuiltin::Sys
{
using namespace yvm;
using namespace ymod;

Module Init();

const ModuleDesc& GetModuleDesc()
{
	static ModuleDesc m;

	if(m.name.empty())
	{
		m.name = "sys";
		m.builtin = true;
		m.initer = Init;
		m.memberTbl["version"] = ModuleMemberDesc{ "version", ModuleMemberDesc::VAR, };
	}
	return m;
}

Module Init()
{
	Module o(&GetModuleDesc());
	o.memberVars["version"] = YArg{Variable::NewStr("ylang 0.0.3"), YEArg::YVar};

	auto argv = Variable::NewList();
	for(size_t i=1; i<g_Args.size(); i++)
	{
		argv->_list->push_back(Variable::NewStr(g_Args[i]));
	}
	o.memberVars["argv"] = YArg{argv, YEArg::YVar};
	o.memberVars["executable"] = YArg{Variable::NewStr(g_Args[0]), YEArg::YVar};
	return o;
}

}
