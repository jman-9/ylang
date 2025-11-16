#pragma once
#include "Primitives.h"


namespace yvm::primitive
{

static const ymod::ModuleDesc& strModDesc = yvm::primitive::Str::GetModuleDesc();
static const ymod::ModuleDesc& listModDesc= yvm::primitive::List::GetModuleDesc();
static const ymod::ModuleDesc& dictModDesc= yvm::primitive::Dict::GetModuleDesc();

const ymod::ModuleDesc* GetModuleDesc(Variable::Type primType)
{
	switch(primType)
	{
	case Variable::STR: return &strModDesc;
	case Variable::LIST: return &listModDesc;
	case Variable::DICT: return &dictModDesc;
	}
	return nullptr;
}

}
