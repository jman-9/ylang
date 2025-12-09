#include "Sys.h"
#include "ylangDefs.h"
#include "util/Args.h"
#include "vm/Variable.h"
#include <format>


namespace ybuiltin::Sys
{
using namespace std;
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
	o.memberVars["argv"] = YArg{argv, YEArg::YVar};

	v = new Variable;
	v->SetStr(g_Args[0]);
	o.memberVars["executable"] = YArg{v, YEArg::YVar};
	return o;
}

}
