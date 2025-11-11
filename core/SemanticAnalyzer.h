#pragma once
#include "TreeNode.h"
#include "Symbol.h"
#include "Error.h"
#include <string>
#include <map>


class SemanticAnalyzer
{
public:
	SemanticAnalyzer();
	~SemanticAnalyzer();

	bool Analyze(const TreeNode& code);

	std::vector<Error> _errors;

protected:
	std::vector<std::map<std::string, Symbol>> _symTbl;


	bool AnalyzeStmt(const TreeNode& stmt);
	bool AnalyzeExp(const TreeNode& stmt);
	bool AnalyzeInclude(const TreeNode& stmt);
	bool AnalyzeFor(const TreeNode& stmt);
	bool AnalyzeIf(const TreeNode& stmt);
	bool AnalyzeFn(const TreeNode& stmt);
	bool AnalyzeCompound(const TreeNode& stmt, const std::vector<Param>& stackVars = std::vector<Param>());
};
