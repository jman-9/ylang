#pragma once
#include "Variable.h"
#include "Token.h"
#include <string>


namespace yvm
{

struct ErrorBase
{
	int srcLine = -1;
	std::string what;
};

struct IndexError : public ErrorBase
{
	enum Type
	{
		NONE,
		NOT_FOUND,
		OUT_OF_RANGE,
		UNSUPPORTED,
	};

	Type type = NONE;
	Variable::Type varType = Variable::NONE;
	std::string varName;
	std::string index;
	int len = 0;

	static IndexError NotFound(Variable::Type varType, std::string varName, std::string index);
	static IndexError OutOfRange(Variable::Type varType, std::string varName, int index, int len);
	static IndexError Unsupported(Variable::Type varType, std::string varName, std::string index);
};


struct TypeError : public ErrorBase
{
	enum Type
	{
		NONE,
		UNSUPPORTED_OPERAND,
		UNSUPPORTED_OPERANDS,
	};

	Type type = NONE;
	EToken tokType = EToken::None;
	Variable::Type lhsType = Variable::NONE;
	Variable::Type rhsType = Variable::NONE;
	std::string lhsName;
	std::string rhsName;

	static TypeError UnsupportedOperand(EToken tokType, Variable::Type unaryType, std::string unaryName);
	static TypeError UnsupportedOperands(EToken tokType, Variable::Type lhsType, std::string lhsName, Variable::Type rhsType, std::string rhsName);
};

struct MemberError : public ErrorBase
{
	enum Type
	{
		NONE,
		NO_MEMBER,
		NOT_MATCHED_PARAMS,
	};

	Type type = NONE;
	Variable::Type ownerType = Variable::NONE;
	std::string ownerName;
	std::string memberName;
	int numPrms = 0;
	int numArgs = 0;

	static MemberError NoMember(Variable::Type ownerType, std::string ownerName, std::string memberName);
	static MemberError NotMatchedParams(Variable::Type ownerType, std::string ownerName, std::string memberName, int numPrms, int numArgs);
};

}
