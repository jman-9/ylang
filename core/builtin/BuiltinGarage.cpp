#include "BuiltinGarage.h"
#include "Math.h"


bool ybuiltin::Garage::RegisterAll(ymod::ModuleManager& retModMgr)
{
	if(!retModMgr.RegisterModule(Math::GetModule())) return false;
	return true;
}
