#pragma once
#include "Bytecode.h"
#include <format>


static char ValKindChar(ERefKind k)
{
	switch(k)
	{
	case ERefKind::Const : return 'c';
	case ERefKind::Reg : return 'r';
	case ERefKind::LocalVar : return 'l';
	case ERefKind::GlobalVar : return 'g';
	case ERefKind::MemberVar : return 'm';
	}
	return ' ';
}
static char ValKindChar(uint8_t k)
{
	return ValKindChar(ERefKind(k));
}


template<EOpcode Op>
void Bytecode::FillBytecode(int ln, int srcLine /* = -1 */)
{
	_code[ln].Fill<Op>();

	if constexpr (Op == EOpcode::Noop)
	{
		_codeStr[ln] = "noop";
	}
	else if constexpr (Op == EOpcode::PushSp)
	{
		_codeStr[ln] = "pushsp";
	}
	else if constexpr (Op == EOpcode::PopSp)
	{
		_codeStr[ln] = "popsp";
	}
	else if constexpr (Op == EOpcode::Ret)
	{
		_codeStr[ln] = "ret";
	}

	if(srcLine > -1)
	{
		_codeStr[ln] += "\t\t" + std::to_string(srcLine);
	}
}

template<class OpType>
void Bytecode::FillBytecode(int ln, const OpType& inst, int srcLine /* = -1 */)
{
	_code[ln].Fill(inst);

	if constexpr (std::is_same_v<Op::Assign, OpType>)
	{
		if(inst.dstKind && inst.src1Kind)
		{
			_codeStr[ln] = std::format("{}{} = {}{}", ValKindChar(inst.dstKind), inst.dst, ValKindChar(inst.src1Kind), inst.src1);

			if(inst.op)
			{
				_codeStr[ln] += std::format(" {} {}{}", Token::TokenString((EToken)inst.op), ValKindChar(inst.src2Kind), inst.src2);
			}
		}
		else if(inst.dstKind && inst.src2Kind)
		{
			_codeStr[ln] = std::format("{}{} = {}{}{}", ValKindChar(inst.dstKind), inst.dst, Token::TokenString((EToken)inst.op), ValKindChar(inst.src2Kind), inst.src2);
		}
		else if(inst.src1Kind && inst.src2Kind)
		{
			_codeStr[ln] = std::format("{}{} {} {}{}", ValKindChar(inst.src1Kind), inst.src1, Token::TokenString((EToken)inst.op), ValKindChar(inst.src2Kind), inst.src2);
		}
		else if(inst.src1Kind)
		{
			_codeStr[ln] = std::format("{}{}{}", ValKindChar(inst.src1Kind), inst.src1, Token::TokenString((EToken)inst.op));
		}
		else if(inst.src2Kind)
		{
			_codeStr[ln] = std::format("{}{}{}", Token::TokenString((EToken)inst.op), ValKindChar(inst.src2Kind), inst.src2);
		}
		else
		{
			throw 'n';
		}
	}
	else if constexpr (std::is_same_v<Op::Jmp, OpType>)
	{
		_codeStr[ln] = std::format("jmp {}", inst.pos);
	}
	else if constexpr (std::is_same_v<Op::Call, OpType>)
	{
		_codeStr[ln] = std::format("{}{} = call {}", ValKindChar(ERefKind::Reg), "l", inst.pos);
	}
	else if constexpr (std::is_same_v<Op::Jz, OpType>)
	{
		_codeStr[ln] = std::format("jz {}{}, {}", ValKindChar(inst.testKind), inst.test, inst.pos);
	}
	else if constexpr (std::is_same_v<Op::ListSet, OpType>)
	{
		_codeStr[ln] = std::format("listset {}{}", ValKindChar(inst.dstKind), inst.dst);
	}
	else if constexpr (std::is_same_v<Op::ListAdd, OpType>)
	{
		_codeStr[ln] = std::format("listadd {}{}, {}{}", ValKindChar(inst.dstKind), inst.dst, ValKindChar(inst.srcKind), inst.src);
	}
	else if constexpr (std::is_same_v<Op::DictSet, OpType>)
	{
		_codeStr[ln] = std::format("dictset {}{}", ValKindChar(inst.dstKind), inst.dst);
	}
	else if constexpr (std::is_same_v<Op::DictAdd, OpType>)
	{
		_codeStr[ln] = std::format("dictadd {}{}, {}{}:{}{}", ValKindChar(inst.dstKind), inst.dst, ValKindChar(inst.keyKind), inst.key, ValKindChar(inst.valKind), inst.val);
	}
	else if constexpr (std::is_same_v<Op::Index, OpType>)
	{
		_codeStr[ln] = std::format("index {}{}[{}{}]", ValKindChar(inst.dstKind), inst.dst, ValKindChar(inst.idxKind), inst.idx);
	}
	else if constexpr (std::is_same_v<Op::LValueIndex, OpType>)
	{
		_codeStr[ln] = std::format("lvalueindex {}{}[{}{}]", ValKindChar(inst.dstKind), inst.dst, ValKindChar(inst.idxKind), inst.idx);
	}
	else if constexpr (std::is_same_v<Op::Invoke, OpType>)
	{
		_codeStr[ln] = std::format("invoke {}{}(...)", ValKindChar(ERefKind::Reg), "l", ValKindChar(inst.dstKind), inst.dst);
	}
	else if constexpr (std::is_same_v<Op::Inc, OpType>)
	{
		_codeStr[ln] = std::format("include {}{}", ValKindChar(ERefKind::Const), inst.inc);
	}
	else if constexpr (std::is_same_v<Op::Jnz, OpType>)
	{
		_codeStr[ln] = std::format("jnz {}{}, {}", ValKindChar(inst.testKind), inst.test, inst.pos);
	}
	else if constexpr (std::is_same_v<Op::NewMod, OpType>)
	{
		_codeStr[ln] = std::format("newmod {}{}(...)", ValKindChar(ERefKind::Reg), "l", ValKindChar(inst.dstKind), inst.dst);
	}
	else if constexpr (std::is_same_v<Op::NewCls, OpType>)
	{
		_codeStr[ln] = std::format("newcls {}{}(...)", ValKindChar(ERefKind::Reg), "l", ValKindChar(inst.dstKind), inst.dst);
	}
	else
	{
		throw 'n';
	}

	if(srcLine > -1)
	{
		_codeStr[ln] += "\t\t" + std::to_string(srcLine);
	}
}

template<EOpcode Op>
int Bytecode::PushBytecode(int srcLine /* = -1 */)
{
	_code.push_back(Instruction());
	_codeStr.push_back("");
	FillBytecode<Op>(_code.size()-1, srcLine);
	return _code.size()-1;
}

template<class OpType>
int Bytecode::PushBytecode(const OpType& inst, int srcLine /* = -1 */)
{
	_code.push_back(Instruction());
	_codeStr.push_back("");
	FillBytecode(_code.size()-1, inst, srcLine);
	return _code.size()-1;
}

int Bytecode::endOfCode() const
{
	return (int)_code.size() - 1;
}

int Bytecode::nextCodeSlot() const
{
	return (int)_code.size();
}
