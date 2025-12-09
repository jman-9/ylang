#pragma once
#include "module/ModuleManager.h"


namespace ybuiltin::Garage
{
	bool RegisterAll(ymod::ModuleManager& retModMgr);
	bool IsBuiltin(const std::string& name);
}
