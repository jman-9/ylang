#pragma once
#include "contract/ycontract.h"
#include <unordered_map>


namespace ymod
{

struct ModuleFuncDesc
{
	std::string name;
	int numPrms;
	//TODO _type list
	YModFn func;
};

struct ModulePropDesc
{
	std::string name;
	//TODO
};


using ModuleFuncTable = std::unordered_map<std::string, ModuleFuncDesc>;
using ModulePropTable = std::unordered_map<std::string, ModulePropDesc>;

struct Module
{
	std::string name;
	ModuleFuncTable funcTbl;
	ModulePropTable propTbl;
};

}
