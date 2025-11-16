#include "Rand.h"
#include "vm/Variable.h"
#include <time.h>


#define RANDOM(__min__, __max__) ((int)(((double)((rand()<<15) | (rand()&0) | rand())) / ((RAND_MAX<<15 | RAND_MAX) + 1) * (((__max__) + 1) - (__min__))) + (__min__))


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
	o.memberVars["version"] = "ylang 0.0.2";
	return o;
}

}
