#pragma once
#include "RuntimeError.h"
#include <format>


namespace yvm
{
using namespace std;


IndexError IndexError::NotFound(Variable::Type varType, std::string varName, std::string index)
{
	IndexError e;
	e.type = NOT_FOUND;
	e.varType = varType;
	e.varName = varName;
	e.index = index;

	e.what = format("'{}': not found in '{}'", index, varName);
	return e;
}
IndexError IndexError::OutOfRange(Variable::Type varType, std::string varName, int index, int len)
{
	IndexError e;
	e.type = OUT_OF_RANGE;
	e.varType = varType;
	e.varName = varName;
	e.index = to_string(index);
	e.len = len;

	e.what = format("'{}': index out of range in '{}(len:{})'", index, varName, len);
	return e;
}
IndexError IndexError::Unsupported(Variable::Type varType, std::string varName, std::string index)
{
	IndexError e;
	e.type = UNSUPPORTED;
	e.varType = varType;
	e.varName = varName;
	e.index = index;

	e.what = format("'{}': index unsupported for '{}'", index, Variable::TypeStr(varType));
	return e;
}

TypeError TypeError::UnsupportedOperand(EToken tokType, Variable::Type unaryType, std::string unaryName)
{
	TypeError e;
	e.type = UNSUPPORTED_OPERAND;
	e.tokType = tokType;
	e.lhsType = unaryType;
	e.lhsName = unaryName;

	e.what = format("'{}': unsupported for '{}'", Variable::TypeStr(unaryType), Token::TokenString(e.tokType));
	return e;
}

TypeError TypeError::UnsupportedOperands(EToken tokType, Variable::Type lhsType, std::string lhsName, Variable::Type rhsType, std::string rhsName)
{
	TypeError e;
	e.type = UNSUPPORTED_OPERANDS;
	e.tokType = tokType;
	e.lhsType = lhsType;
	e.lhsName = lhsName;
	e.rhsType = rhsType;
	e.rhsName = rhsName;

	e.what = format("'{}','{}': unsupported for '{}'", Variable::TypeStr(e.lhsType), Variable::TypeStr(e.rhsType), Token::TokenString(e.tokType));
	return e;
}


MemberError MemberError::NoMember(Variable::Type ownerType, std::string ownerName, std::string memberName)
{
	MemberError e;
	e.type = NO_MEMBER;
	e.ownerType = ownerType;
	e.ownerName = ownerName;
	e.memberName = memberName;
	if(ownerName.empty())
	{
		e.what = format("'{}' type has no member '{}'", Variable::TypeStr(e.ownerType), e.memberName);
	}
	else
	{
		e.what = format("'{}' object has no member '{}'", e.ownerName, e.memberName);
	}
	return e;
}

MemberError MemberError::NotMatchedParams(Variable::Type ownerType, std::string ownerName, std::string memberName, int numPrms, int numArgs)
{
	MemberError e;
	e.type = NOT_MATCHED_PARAMS;
	e.ownerType = ownerType;
	e.ownerName = ownerName;
	e.memberName = memberName;

	/*
	e.what = format("'{}' type has no member '{}'", (int)e.ownerType, e.memberName);
	}
	else
	{
		e.what = format("'{}' object has no member '{}'", e.ownerName, e.memberName);
	}*/
	return e;
}

}
