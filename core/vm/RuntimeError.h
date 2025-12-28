#pragma once
#include "Variable.h"
#include "Token.h"
#include <format>
#include <filesystem>
#include <string>


namespace yvm
{

#define INTERNALERR(__msg__) do { auto e = RuntimeError::Internal(__msg__); e._internalPath = std::filesystem::path(__FILE__).filename().string(); e._internalLine = __LINE__; throw e; } while(0)

#define INTERNALERR_NUMARGS(__prms__, __args__) INTERNALERR(std::format("not matched numbers of params- prms(min):{}, args:{}", __prms__, __args__))

#define INTERNALERR_TYPE(__need__, __cur__) INTERNALERR(std::format("'{}': unmatched type to '{}'", __cur__, __need__))

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
		INCORRECT_PARAM,
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

	static RuntimeError Internal(std::string msg);
	static RuntimeError NotFound(Variable::Type varType, std::string varName, std::string index);
	static RuntimeError OutOfRange(Variable::Type varType, std::string varName, int index, int len);
	static RuntimeError UnsupportedType(Variable::Type varType, std::string varName, std::string index);
	static RuntimeError UnsupportedOperand(EToken tokType, Variable::Type unaryType, std::string unaryName);
	static RuntimeError UnsupportedOperands(EToken tokType, Variable::Type lhsType, std::string lhsName, Variable::Type rhsType, std::string rhsName);
	static RuntimeError IncorrectParam(Variable::Type input, std::vector<Variable::Type> paramTypes, int nTh);
	static RuntimeError IncorrectParam(Variable::Type input, Variable::Type param, int nTh);
	static RuntimeError DivideByZero();
	static RuntimeError NoMember(Variable::Type ownerType, std::string ownerName, std::string memberName);
	static RuntimeError NotMatchedParams(Variable::Type ownerType, std::string ownerName, std::string memberName, int numPrms, int numArgs);
};

}
