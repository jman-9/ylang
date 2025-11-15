#pragma once
#include "contract/ycontract.h"
#include <unordered_map>


namespace ymod
{

struct ModuleFuncDesc
{
	std::string name;
	bool needSelf = false;
	int numPrms = 0;	// todo clarify min params
	//TODO _type list
	YModFn func = nullptr;
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
	bool builtin = false;
	YModFn newer = nullptr;
	YModFn deleter = nullptr;
	ModuleFuncTable funcTbl;
	ModulePropTable propTbl;

	bool IsNull() const;
};

}
