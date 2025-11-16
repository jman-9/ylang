#include "BuiltinGarage.h"
#include "Sys.h"
#include "Math.h"
#include "Rand.h"


bool ybuiltin::Garage::RegisterAll(ymod::ModuleManager& retModMgr)
{
	if(!retModMgr.RegisterModuleDesc(Sys::GetModuleDesc())) return false;
	if(!retModMgr.RegisterModuleDesc(Math::GetModuleDesc())) return false;
	if(!retModMgr.RegisterModuleDesc(Rand::GetModuleDesc())) return false;
	return true;
}
