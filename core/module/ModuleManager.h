#pragma once
#include "Module.h"


namespace ymod
{

class ModuleManager
{
public:
	void IncludeModule(const Module& module);

protected:
	std::unordered_map<std::string, Module> _modMap;
};

}
