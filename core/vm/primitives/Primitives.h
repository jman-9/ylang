#pragma once
#include "Str.h"
#include "List.h"
#include "Dict.h"

namespace yvm::primitive
{

const ymod::ModuleDesc* GetModuleDesc(Variable2::Type primType);

}

#include "Primitives.inl"
