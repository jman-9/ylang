#pragma once
#include "Instruction.h"
#include <type_traits>


inline EOpcode Instruction::GetKind() const
{
	return (EOpcode)kind;
}

inline bool Instruction::operator==(EOpcode op) const
{
	return GetKind() == op;
}


template<EOpcode Op>
void Instruction::Fill()
{
	if constexpr (Op == EOpcode::Noop)
	{
	}
	else if constexpr (Op == EOpcode::PushSp)
	{
	}
	else if constexpr (Op == EOpcode::PopSp)
	{
	}
	else if constexpr (Op == EOpcode::Ret)
	{
	}
	else
	{
		throw 'n';
	}

	code.clear();
	kind = (uint8_t)Op;
}

template<class InstType>
void Instruction::Fill(const InstType& inst)
{
	if constexpr (std::is_same_v<Inst::Assign, InstType>)
	{
		kind = (uint8_t)EOpcode::Assign;
	}
	else if constexpr (std::is_same_v<Inst::Jmp, InstType>)
	{
		kind = (uint8_t)EOpcode::Jmp;
	}
	else if constexpr (std::is_same_v<Inst::Invoke, InstType>)
	{
		kind = (uint8_t)EOpcode::Invoke;
	}
	else if constexpr (std::is_same_v<Inst::Jz, InstType>)
	{
		kind = (uint8_t)EOpcode::Jz;
	}
	else
	{
		throw 'n';
	}

	code.resize(sizeof(inst));
	memcpy(code.data(), &inst, sizeof(inst));
}
