#pragma once
#include "TreeNode.h"
#include "Symbol.h"
#include <string>
#include <vector>
#include <map>


struct Instruction
{
	std::vector<uint8_t> code;
	std::string codeStr;
};


class BytecodeBuilder
{
public:
	BytecodeBuilder(const TreeNode& code);
	~BytecodeBuilder();

	bool Build();

protected:
	const TreeNode& _code;
	uint32_t _reg;
	std::vector<Instruction> _bytecode;

	std::vector<std::map<std::string, Symbol>> _symTbl;

	bool BuildStmt(const TreeNode& stmt);
	bool BuildFor(const TreeNode& stmt);
	bool BuildIf(const TreeNode& stmt);
	bool BuildFn(const TreeNode& stmt);
	bool BuildCompound(const TreeNode& stmt);
	bool BuildExp(const TreeNode& stmt, bool root);
	bool BuildInvoke(const TreeNode& stmt);
};
