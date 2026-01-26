#pragma once
#include "Primitives.h"


namespace yvm::primitive
{

inline const ymod::ModuleDesc* GetModuleDesc(Variable::Type primType)
{
	static const ymod::ModuleDesc& strModDesc = yvm::primitive::Str::GetModuleDesc();
	static const ymod::ModuleDesc& listModDesc= yvm::primitive::List::GetModuleDesc();
	static const ymod::ModuleDesc& dictModDesc= yvm::primitive::Dict::GetModuleDesc();
	static const ymod::ModuleDesc& bytesModDesc= yvm::primitive::Bytes::GetModuleDesc();

	switch(primType)
	{
	case Variable::STR: return &strModDesc;
	case Variable::LIST: return &listModDesc;
	case Variable::DICT: return &dictModDesc;
	case Variable::BYTES: return &bytesModDesc;
	}
	return nullptr;
}

}
