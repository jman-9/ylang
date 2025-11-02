#pragma once
#include "TreeNode.h"
#include "Symbol.h"
#include "Error.h"
#include <string>
#include <map>


class SemanticAnalyzer
{
public:
	SemanticAnalyzer(const TreeNode& code);
	~SemanticAnalyzer();

	bool Analyze();

	std::vector<Error> _errors;

protected:
	const TreeNode& _code;

	std::vector<std::map<std::string, Symbol>> _symTbl;


	bool AnalyzeStmt(const TreeNode& stmt);
	bool AnalyzeExp(const TreeNode& stmt);
	bool AnalyzeFor(const TreeNode& stmt);
	bool AnalyzeIf(const TreeNode& stmt);
	bool AnalyzeFn(const TreeNode& stmt);
	bool AnalyzeCompound(const TreeNode& stmt, const std::vector<Param>& stackVars = std::vector<Param>());
};
