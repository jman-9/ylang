#pragma once
#include "Bytecode.h"
#include <stdexcept>
#include <format>


static std::string_view ValKindChar(ERefKind k)
{
	switch(k)
	{
	case ERefKind::Literal : return "li";
	case ERefKind::Const : return "c";
	case ERefKind::Reg : return "r";
	case ERefKind::LocalVar : return "l";
	case ERefKind::GlobalVar : return "g";
	case ERefKind::FieldVar : return "f";
	case ERefKind::MemberFunc : return "m";
	}
	return " ";
}
static std::string_view ValKindChar(uint8_t k)
{
	return ValKindChar(ERefKind(k));
}


template<EOpcode Op>
void Bytecode::FillBytecode(int ln, int srcLine /* = -1 */)
{
	_code[ln].Fill<Op>();

	if constexpr (Op == EOpcode::Noop)
	{
		_codeStrs[ln] = "noop";
	}
	else if constexpr (Op == EOpcode::PushSp)
	{
		_codeStrs[ln] = "pushsp";
	}
	else if constexpr (Op == EOpcode::PopSp)
	{
		_codeStrs[ln] = "popsp";
	}
	else if constexpr (Op == EOpcode::Ret)
	{
		_codeStrs[ln] = "ret";
	}

	if(srcLine > -1)
	{
		_codeStrs[ln] += "\t\t" + std::to_string(srcLine);
		_srcLines[ln] = srcLine;
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
			_codeStrs[ln] = std::format("{}{} = {}{}", ValKindChar(inst.dstKind), inst.dst, ValKindChar(inst.src1Kind), inst.src1);

			if(inst.op)
			{
				_codeStrs[ln] += std::format(" {} {}{}", Token::TokenString((EToken)inst.op), ValKindChar(inst.src2Kind), inst.src2);
			}
		}
		else if(inst.dstKind && inst.src2Kind)
		{
			_codeStrs[ln] = std::format("{}{} = {}{}{}", ValKindChar(inst.dstKind), inst.dst, Token::TokenString((EToken)inst.op), ValKindChar(inst.src2Kind), inst.src2);
		}
		else if(inst.src1Kind && inst.src2Kind)
		{
			_codeStrs[ln] = std::format("{}{} {} {}{}", ValKindChar(inst.src1Kind), inst.src1, Token::TokenString((EToken)inst.op), ValKindChar(inst.src2Kind), inst.src2);
		}
		else if(inst.src1Kind)
		{
			_codeStrs[ln] = std::format("{}{}{}", ValKindChar(inst.src1Kind), inst.src1, Token::TokenString((EToken)inst.op));
		}
		else if(inst.src2Kind)
		{
			_codeStrs[ln] = std::format("{}{}{}", Token::TokenString((EToken)inst.op), ValKindChar(inst.src2Kind), inst.src2);
		}
		else
		{
			throw std::logic_error(std::format("FillBytecode - line:{} srcLine:{}, op:Assign", ln, srcLine));
		}
	}
	else if constexpr (std::is_same_v<Op::Jmp, OpType>)
	{
		_codeStrs[ln] = std::format("jmp {}", inst.pos);
	}
	else if constexpr (std::is_same_v<Op::Call, OpType>)
	{
		_codeStrs[ln] = std::format("{}{} = call {}", ValKindChar(ERefKind::Reg), "l", inst.pos);
	}
	else if constexpr (std::is_same_v<Op::Jz, OpType>)
	{
		_codeStrs[ln] = std::format("jz {}{}, {}", ValKindChar(inst.testKind), inst.test, inst.pos);
	}
	else if constexpr (std::is_same_v<Op::ListSet, OpType>)
	{
		_codeStrs[ln] = std::format("listset {}{}", ValKindChar(inst.dstKind), inst.dst);
	}
	else if constexpr (std::is_same_v<Op::ListAdd, OpType>)
	{
		_codeStrs[ln] = std::format("listadd {}{}, {}{}", ValKindChar(inst.dstKind), inst.dst, ValKindChar(inst.srcKind), inst.src);
	}
	else if constexpr (std::is_same_v<Op::DictSet, OpType>)
	{
		_codeStrs[ln] = std::format("dictset {}{}", ValKindChar(inst.dstKind), inst.dst);
	}
	else if constexpr (std::is_same_v<Op::DictAdd, OpType>)
	{
		_codeStrs[ln] = std::format("dictadd {}{}, {}{}:{}{}", ValKindChar(inst.dstKind), inst.dst, ValKindChar(inst.keyKind), inst.key, ValKindChar(inst.valKind), inst.val);
	}
	else if constexpr (std::is_same_v<Op::Index, OpType>)
	{
		_codeStrs[ln] = std::format("index {}{}[{}{}]", ValKindChar(inst.dstKind), inst.dst, ValKindChar(inst.idxKind), inst.idx);
	}
	else if constexpr (std::is_same_v<Op::LValueIndex, OpType>)
	{
		_codeStrs[ln] = std::format("lvalueindex {}{}[{}{}]", ValKindChar(inst.dstKind), inst.dst, ValKindChar(inst.idxKind), inst.idx);
	}
	else if constexpr (std::is_same_v<Op::Invoke, OpType>)
	{
		_codeStrs[ln] = std::format("{}{} = invoke {}{}(...)", ValKindChar(inst.dstKind), inst.dst, ValKindChar(inst.dstKind), inst.dst);
	}
	else if constexpr (std::is_same_v<Op::Inc, OpType>)
	{
		_codeStrs[ln] = std::format("include {}{}", ValKindChar(ERefKind::Const), inst.inc);
	}
	else if constexpr (std::is_same_v<Op::Jnz, OpType>)
	{
		_codeStrs[ln] = std::format("jnz {}{}, {}", ValKindChar(inst.testKind), inst.test, inst.pos);
	}
	else if constexpr (std::is_same_v<Op::NewMod, OpType>)
	{
		_codeStrs[ln] = std::format("{}{} = newmod {}{}(...)", ValKindChar(inst.dstKind), inst.dst, ValKindChar(inst.nameKind), inst.name);
	}
	else if constexpr (std::is_same_v<Op::NewCls, OpType>)
	{
		_codeStrs[ln] = std::format("{}{} = newcls {}{}(...)", ValKindChar(inst.dstKind), inst.dst, ValKindChar(inst.nameKind), inst.name);
	}
	else if constexpr (std::is_same_v<Op::LValueField, OpType>)
	{
		_codeStrs[ln] = std::format("lvaluefield {}{}.{}{}", ValKindChar(inst.dstKind), inst.dst, ValKindChar(inst.fieldKind), inst.field);
	}
	else
	{
		throw std::logic_error(std::format("FillBytecode - line:{} srcLine:{} op:unknown", ln, srcLine));
	}

	if(srcLine > -1)
	{
		_codeStrs[ln] += "\t\t" + std::to_string(srcLine);
		_srcLines[ln] = srcLine;
	}
}

template<EOpcode Op>
int Bytecode::PushBytecode(int srcLine /* = -1 */)
{
	_code.push_back(Instruction());
	_srcLines.push_back(-1);
	_codeStrs.push_back("");
	FillBytecode<Op>(_code.size()-1, srcLine);
	return _code.size()-1;
}

template<class OpType>
int Bytecode::PushBytecode(const OpType& inst, int srcLine /* = -1 */)
{
	_code.push_back(Instruction());
	_srcLines.push_back(srcLine);
	_codeStrs.push_back("");
	FillBytecode(_code.size()-1, inst, srcLine);
	return _code.size()-1;
}


bool Bytecode::empty() const
{
	return _code.empty();
}

int Bytecode::endOfCode() const
{
	return (int)_code.size() - 1;
}

int Bytecode::nextCodeSlot() const
{
	return (int)_code.size();
}
