#include "RuntimeError.h"
#include <format>


namespace yvm
{
using namespace std;

/*
void RuntimeError::SetSrcInfo(std::string srcPath, int srcLine, int bytecodeLine)
{
	_srcPath = srcPath;
	_srcLine = srcLine;
	_bytecodeLine = bytecodeLine;
}
void RuntimeError::SetDebugInfo(string file, int fileLine)
{
	_file = file;
	_bytecodeLine =
	_srcLine = srcLine;
	_bytecodeLine = bytecodeLine;
}*/

RuntimeError RuntimeError::Internal(std::string msg)
{
	RuntimeError e;
	e._type = LOGIC;
	e._msg = msg;
	return e;
}

RuntimeError RuntimeError::NotFound(Variable::Type varType, std::string varName, std::string index)
{
	RuntimeError e;
	e._type = INDEX;
	e._code = NOT_FOUND;
	e._msg = format("'{}': not found in '{}({})'", index, varName, Variable::TypeStr(varType));
	return e;
}

RuntimeError RuntimeError::OutOfRange(Variable::Type varType, std::string varName, int index, int len)
{
	RuntimeError e;
	e._type = INDEX;
	e._code = OUT_OF_RANGE;
	e._msg = format("'{}': index out of range in '{}(type:{}, len:{})'", index, varName, Variable::TypeStr(varType), len);
	return e;
}

RuntimeError RuntimeError::UnsupportedType(Variable::Type varType, std::string varName, std::string index)
{
	RuntimeError e;
	e._type = INDEX;
	e._code = UNSUPPORTED_TYPE;
	e._msg = format("'{}': index unsupported for '{}'", index, Variable::TypeStr(varType));
	return e;
}

RuntimeError RuntimeError::UnsupportedOperand(EToken tokType, Variable::Type unaryType, std::string unaryName)
{
	RuntimeError e;
	e._type = TYPE;
	e._code = UNSUPPORTED_OPERAND;
	e._msg = format("'{}': unsupported for '{}'", Variable::TypeStr(unaryType), Token::TokenString(tokType));
	return e;
}

RuntimeError RuntimeError::UnsupportedOperands(EToken tokType, Variable::Type lhsType, std::string lhsName, Variable::Type rhsType, std::string rhsName)
{
	RuntimeError e;
	e._type = TYPE;
	e._code = UNSUPPORTED_OPERANDS;
	e._msg = format("'{}','{}': unsupported for '{}'", Variable::TypeStr(lhsType), Variable::TypeStr(rhsType), Token::TokenString(tokType));
	return e;
}

yvm::RuntimeError RuntimeError::IncorrectParam(Variable::Type input, std::vector<Variable::Type> paramTypes, int nTh)
{
	RuntimeError e;
	e._type = TYPE;
	e._code = INCORRECT_PARAM;
	string paramTypeStr;
	for(auto t : paramTypes)
	{
		paramTypeStr += Variable::TypeStr(t);
		paramTypeStr += ' or ';
	}
	if(paramTypeStr.ends_with('or '))
		paramTypeStr.resize(paramTypeStr.size() - strlen(" or "));

	e._msg = format("'{}': incorrect param {} (expected '{}')", Variable::TypeStr(input), nTh, paramTypeStr);
	return e;
}
yvm::RuntimeError RuntimeError::IncorrectParam(Variable::Type input, Variable::Type param, int nTh)
{
	return IncorrectParam(input, vector<Variable::Type>{param}, nTh);
}

yvm::RuntimeError RuntimeError::DivideByZero()
{
	RuntimeError e;
	e._type = TYPE;
	e._code = DIVIDE_BY_ZERO;
	e._msg = format("divide by zero");
	return e;
}

RuntimeError RuntimeError::NoMember(Variable::Type ownerType, std::string ownerName, std::string memberName)
{
	RuntimeError e;
	e._type = TYPE;
	e._code = NO_MEMBER;
	if(ownerName.empty())
	{
		e._msg = format("'{}' type has no member '{}'", Variable::TypeStr(ownerType), memberName);
	}
	else
	{
		e._msg = format("'{}' object has no member '{}'", ownerName, memberName);
	}
	return e;
}

RuntimeError RuntimeError::NotMatchedParams(Variable::Type ownerType, std::string ownerName, std::string memberName, int numPrms, int numArgs)
{
	RuntimeError e;
	e._type = TYPE;
	e._code = NOT_MATCHED_PARAMS;
	/*TODO
	e.what = format("'{}' type has no member '{}'", (int)e.ownerType, e.memberName);
	}
	else
	{
		e.what = format("'{}' object has no member '{}'", e.ownerName, e.memberName);
	}*/
	return e;
}

string RuntimeError::ToStr() const
{
	string stp = "UnknownError";
	switch(_type)
	{
	case CLASS_NONE: break;
	case LOGIC: stp = "LogicError"; break;
	case INDEX: stp = "IndexError"; break;
	case TYPE: stp = "TypeError"; break;
	case MEMBER: stp = "MemberError"; break;
	}

	if(_type == LOGIC)
	{
		return format("File: {}\nLine: {}\nInternalFile: {}\nInternalLine: {}\n{}: {}", _srcPath, _srcLine, _internalPath, _internalLine, stp, _msg);
	}
	else
	{
		return format("File: {}\nLine: {}\n{}: {}", _srcPath, _srcLine, stp, _msg);
	}
}

}
