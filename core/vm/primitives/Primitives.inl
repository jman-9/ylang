#pragma once
#include "Primitives.h"


namespace yvm::primitive
{

static const ymod::Module& strMod = yvm::primitive::Str::GetModule();
static const ymod::Module& listMod= yvm::primitive::List::GetModule();
static const ymod::Module& dictMod= yvm::primitive::Dict::GetModule();

const ymod::Module* GetModule(Variable::Type primType)
{
	switch(primType)
	{
	case Variable::STR: return &strMod;
	case Variable::LIST: return &listMod;
	case Variable::DICT: return &dictMod;
	}
	return nullptr;
}

}
