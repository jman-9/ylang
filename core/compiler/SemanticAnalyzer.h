#pragma once
#include "TreeNode.h"
#include "Symbol.h"
#include "Error.h"
#include "NamespaceUtil.h"
#include "ScopeManager.h"
#include <string>
#include <map>


namespace ycom
{

class SemanticAnalyzer
{
public:
	SemanticAnalyzer();
	~SemanticAnalyzer();

	bool Analyze(const TreeNode& code);

	std::vector<Error> _errors;

protected:
	std::vector<std::map<std::string, Symbol>> _symTbl;
	ScopeManager _scopeMgr;

	NamespaceUtil::Context _nsCtx;
	NamespaceUtil::Tracker _nsTracker;

	void OpenCompound();
	void CloseCompound();

	bool AnalyzeStmt(const TreeNode& stmt);
	bool AnalyzeExp(const TreeNode& stmt);
	bool AnalyzeInclude(const TreeNode& stmt);
	bool AnalyzeFor(const TreeNode& stmt);
	bool AnalyzeIf(const TreeNode& stmt);
	bool AnalyzeFn(const TreeNode& stmt);
	bool AnalyzeCompound(const TreeNode& stmt);
	bool AnalyzeClass(const TreeNode& stmt);

	bool CanBeLValue(const TreeNode& stmt);
};

}
