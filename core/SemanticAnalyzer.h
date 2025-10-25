#pragma once
#include "TreeNode.h"
#include "Symbol.h"
#include <string>
#include <map>


class SemanticAnalyzer
{
public:
	SemanticAnalyzer(const TreeNode& code);
	~SemanticAnalyzer();

	bool Analyze();

protected:
	const TreeNode& _code;

	int scope;
	std::vector<std::map<std::string, Symbol>> _symTbl;


	bool AnalyzeStmt(TreeNode* stmt);
	bool AnalyzeExp(TreeNode* stmt);
	bool AnalyzeFor(TreeNode* stmt);
	bool AnalyzeIf(TreeNode* stmt);
	bool AnalyzeFn(TreeNode* stmt);
	bool AnalyzeCompound(TreeNode* stmt, const std::vector<Param>& stackVars = std::vector<Param>());
};
