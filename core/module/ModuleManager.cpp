#include "ModuleManager.h"

namespace ymod
{

bool ModuleManager::RegisterModule(const Module& module)
{
	if(_modMap.contains(module.name))
		return false;

	_modMap[module.name] = module;
	return true;
}

const ymod::Module& ModuleManager::GetModule(const std::string& name) const
{
	static Module s_emptyModule;

	auto found = _modMap.find(name);
	if(found == _modMap.end())
		return s_emptyModule;

	return found->second;
}

}
