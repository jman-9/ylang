#pragma once
#include "Symbol.h"
#include "Instruction.h"

struct Bytecode
{
	std::vector<std::string> _codeStr;
	std::vector<Instruction> _code;

	template<EOpcode Op>
	void FillBytecode(int ln, int srcLine = -1);
	template<class OpType>
	void FillBytecode(int ln, const OpType& inst, int srcLine = -1);
	template<EOpcode Op>
	int PushBytecode(int srcLine = -1);
	template<class OpType>
	int PushBytecode(const OpType& inst, int srcLine = -1);

	inline int endOfCode() const;
	inline int nextCodeSlot() const;
};

#include "Bytecode.inl"
