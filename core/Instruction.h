#pragma once
#include "InstructionSet.h"
#include <stdint.h>
#include <vector>
#include <string>

struct Instruction
{
	uint8_t kind = (uint8_t)EOpcode::Noop;
	std::vector<uint8_t> code;

	EOpcode GetKind() const;
	bool operator==(EOpcode op) const;

	template<EOpcode Op>
	void Fill();
	template<class InstType>
	void Fill(const InstType& inst);
};

#include "Instruction.inl"
