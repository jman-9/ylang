#pragma once
#include "Module.h"


namespace ymod
{

class ModuleManager
{
public:
	bool RegisterModuleDesc(const ModuleDesc& modDesc);
	const ModuleDesc& GetModuleDesc(const std::string& name) const;

protected:
	std::unordered_map<std::string, ModuleDesc> _modDescMap;
};

}
