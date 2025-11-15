#include "BuiltinGarage.h"
#include "Math.h"
#include "InstTest.h"


bool ybuiltin::Garage::RegisterAll(ymod::ModuleManager& retModMgr)
{
	if(!retModMgr.RegisterModule(Math::GetModule())) return false;
	if(!retModMgr.RegisterModule(InstTest::GetModule())) return false;
	return true;
}
