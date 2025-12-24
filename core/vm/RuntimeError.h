#pragma once
#include "Variable.h"
#include "Token.h"
#include <string>


namespace yvm
{

struct RuntimeError
{
	enum Type
	{
		CLASS_NONE,
		LOGIC,
		INDEX,
		TYPE,
		MEMBER,
	};

	enum Code
	{
		CODE_NONE,
		INTERNAL,
		NOT_FOUND,
		OUT_OF_RANGE,
		UNSUPPORTED_TYPE,
		UNSUPPORTED_OPERAND,
		UNSUPPORTED_OPERANDS,
		DIVIDE_BY_ZERO,
		NO_MEMBER,
		NOT_MATCHED_PARAMS,
	};

	Type _type;
	Code _code;

	std::string _msg;

	std::string _srcPath;
	int _srcLine = -1;
	int _bytecodeLine = -1;

	std::string _internalPath;
	int _internalLine = -1;

	std::string ToStr() const;

	//void SetSrcInfo(std::string srcPath, int srcLine, int bytecodeLine);
	//void SetDebugInfo(std::string internalPath, int Line);

	static RuntimeError Internal(std::string msg);
	static RuntimeError NotFound(Variable::Type varType, std::string varName, std::string index);
	static RuntimeError OutOfRange(Variable::Type varType, std::string varName, int index, int len);
	static RuntimeError UnsupportedType(Variable::Type varType, std::string varName, std::string index);
	static RuntimeError UnsupportedOperand(EToken tokType, Variable::Type unaryType, std::string unaryName);
	static RuntimeError UnsupportedOperands(EToken tokType, Variable::Type lhsType, std::string lhsName, Variable::Type rhsType, std::string rhsName);
	static RuntimeError DivideByZero();
	static RuntimeError NoMember(Variable::Type ownerType, std::string ownerName, std::string memberName);
	static RuntimeError NotMatchedParams(Variable::Type ownerType, std::string ownerName, std::string memberName, int numPrms, int numArgs);
};

}
