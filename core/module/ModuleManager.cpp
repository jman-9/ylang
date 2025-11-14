#include "ModuleManager.h"

namespace yvm
{

void ModuleManager::IncludeModule(const Module& module)
{
	_modMap[module.name] = module;
}

}
