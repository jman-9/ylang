#pragma once
#include "Module.h"


namespace yvm
{

class ModuleManager
{
public:
	void IncludeModule(const Module& module);

protected:
	std::unordered_map<std::string, Module> _modMap;
};

}
