#pragma once
#include "contract/ycontract.h"
#include <unordered_map>


namespace ymod
{

struct ModuleMemberDesc
{
	enum Kind
	{
		VAR,
		FUNC,
	};

	std::string name;
	Kind kind;
	bool needSelf = false;
	int numPrms = 0;	// todo clarify min params
	//TODO _type list
	YModFn func = nullptr;
};

using ModuleMemberTable = std::unordered_map<std::string, ModuleMemberDesc>;
using ModuleMemberVarTable = std::unordered_map<std::string, YArg>;


struct ModuleDesc;
struct Module
{
	const ModuleDesc* modDesc = nullptr;
	ModuleMemberVarTable memberVars;
};

using YModIniter = Module (*)();

struct ModuleDesc
{
	std::string name;
	bool builtin = false;
	YModIniter initer = nullptr;
	YModFn newer = nullptr;
	YModFn deleter = nullptr;
	ModuleMemberTable memberTbl;

	bool IsNull() const;
};

}
