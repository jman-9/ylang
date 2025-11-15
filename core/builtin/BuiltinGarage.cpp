#include "BuiltinGarage.h"
#include "Math.h"
#include "Rand.h"


bool ybuiltin::Garage::RegisterAll(ymod::ModuleManager& retModMgr)
{
	if(!retModMgr.RegisterModule(Math::GetModule())) return false;
	if(!retModMgr.RegisterModule(Rand::GetModule())) return false;
	return true;
}
