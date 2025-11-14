#pragma once
#include "Module.h"


namespace ymod
{

class ModuleManager
{
public:
	bool RegisterModule(const Module& module);
	const Module& GetModule(const std::string& name) const;

protected:
	std::unordered_map<std::string, Module> _modMap;
};

}
