#pragma once
#include "Primitives.h"


namespace yvm::primitive
{

static const ymod::ModuleDesc& strModDesc = yvm::primitive::Str::GetModuleDesc();
static const ymod::ModuleDesc& listModDesc= yvm::primitive::List::GetModuleDesc();
static const ymod::ModuleDesc& dictModDesc= yvm::primitive::Dict::GetModuleDesc();

const ymod::ModuleDesc* GetModuleDesc(Variable2::Type primType)
{
	switch(primType)
	{
	case Variable2::STR: return &strModDesc;
	case Variable2::LIST: return &listModDesc;
	case Variable2::DICT: return &dictModDesc;
	}
	return nullptr;
}

}
