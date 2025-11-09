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

template<class OpType>
void Instruction::Fill(const OpType& inst)
{
	if constexpr (std::is_same_v<Op::Assign, OpType>)
	{
		kind = (uint8_t)EOpcode::Assign;
	}
	else if constexpr (std::is_same_v<Op::Jmp, OpType>)
	{
		kind = (uint8_t)EOpcode::Jmp;
	}
	else if constexpr (std::is_same_v<Op::Invoke, OpType>)
	{
		kind = (uint8_t)EOpcode::Invoke;
	}
	else if constexpr (std::is_same_v<Op::Jz, OpType>)
	{
		kind = (uint8_t)EOpcode::Jz;
	}
	else if constexpr (std::is_same_v<Op::ListSet, OpType>)
	{
		kind = (uint8_t)EOpcode::ListSet;
	}
	else if constexpr (std::is_same_v<Op::ListAdd, OpType>)
	{
		kind = (uint8_t)EOpcode::ListAdd;
	}
	else if constexpr (std::is_same_v<Op::DictSet, OpType>)
	{
		kind = (uint8_t)EOpcode::DictSet;
	}
	else if constexpr (std::is_same_v<Op::DictAdd, OpType>)
	{
		kind = (uint8_t)EOpcode::DictAdd;
	}
	else if constexpr (std::is_same_v<Op::Index, OpType>)
	{
		kind = (uint8_t)EOpcode::Index;
	}
	else if constexpr (std::is_same_v<Op::LValueIndex, OpType>)
	{
		kind = (uint8_t)EOpcode::LValueIndex;
	}
	else if constexpr (std::is_same_v<Op::Call, OpType>)
	{
		kind = (uint8_t)EOpcode::Call;
	}
	else
	{
		throw 'n';
	}

	code.resize(sizeof(inst));
	memcpy(code.data(), &inst, sizeof(inst));
}
