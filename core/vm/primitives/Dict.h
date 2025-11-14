#pragma once
#include "contract/ycontract.h"
#include "module/Module.h"

namespace yvm::primitive::Dict
{

inline const ymod::Module& GetModule();

}

#include "Dict.inl"
