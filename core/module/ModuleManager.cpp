#include "ModuleManager.h"

namespace ymod
{

bool ModuleManager::RegisterModuleDesc(const ModuleDesc& modDesc)
{
	if(_modDescMap.contains(modDesc.name))
		return false;

	_modDescMap[modDesc.name] = modDesc;
	return true;
}

const ymod::ModuleDesc& ModuleManager::GetModuleDesc(const std::string& name) const
{
	static ModuleDesc s_emptyModuleDesc;

	auto found = _modDescMap.find(name);
	if(found == _modDescMap.end())
		return s_emptyModuleDesc;

	return found->second;
}

}
