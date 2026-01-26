#pragma once
#include "Str.h"
#include "List.h"
#include "Dict.h"
#include "Bytes.h"

namespace yvm::primitive
{

inline const ymod::ModuleDesc* GetModuleDesc(Variable::Type primType);

}

#include "Primitives.inl"
