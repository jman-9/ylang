#pragma once
#include "Str.h"
#include "List.h"
#include "Dict.h"

namespace yvm::primitive
{

const ymod::Module* GetModule(Variable::Type primType);

}

#include "Primitives.inl"
