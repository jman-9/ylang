#pragma once
#include "Instruction.h"
#include <string.h>
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
	if constexpr (std::is_same_v<Op::Assign, InstType>)
	{
		kind = (uint8_t)EOpcode::Assign;
	}
	else if constexpr (std::is_same_v<Op::Jmp, InstType>)
	{
		kind = (uint8_t)EOpcode::Jmp;
	}
	else if constexpr (std::is_same_v<Op::Invoke, InstType>)
	{
		kind = (uint8_t)EOpcode::Invoke;
	}
	else if constexpr (std::is_same_v<Op::Jz, InstType>)
	{
		kind = (uint8_t)EOpcode::Jz;
	}
	else if constexpr (std::is_same_v<Op::ListAdd, InstType>)
	{
		kind = (uint8_t)EOpcode::ListAdd;
	}
	else if constexpr (std::is_same_v<Op::ListSet, InstType>)
	{
		kind = (uint8_t)EOpcode::ListSet;
	}
	else if constexpr (std::is_same_v<Op::Index, InstType>)
	{
		kind = (uint8_t)EOpcode::Index;
	}
	else if constexpr (std::is_same_v<Op::LValueIndex, InstType>)
	{
		kind = (uint8_t)EOpcode::LValueIndex;
	}
	else
	{
		throw 'n';
	}

	code.resize(sizeof(inst));
	memcpy(code.data(), &inst, sizeof(inst));
}
