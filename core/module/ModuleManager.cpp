#include "ModuleManager.h"

namespace ymod
{

void ModuleManager::IncludeModule(const Module& module)
{
	_modMap[module.name] = module;
}

}
