#pragma once
#include "TreeNode.h"
#include "Symbol.h"
#include "Error.h"
#include "NamespaceUtil.h"
#include "ScopeManager.h"
#include <string>
#include <map>
#include <unordered_set>


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

	void OpenScope(ScopeManager::ScopeType type);
	void CloseScope();
	void OpenCompound();
	void CloseCompound();

	bool CanBeLValue(const TreeNode& stmt);

	bool AnalyzeStmt(const TreeNode& stmt, const std::unordered_set<EToken>& inSet);
	bool AnalyzeInclude(const TreeNode& stmt);
	bool AnalyzeIf(const TreeNode& stmt, const std::unordered_set<EToken>& inSet);
	bool AnalyzeFor(const TreeNode& stmt, const std::unordered_set<EToken>& inSet);
	bool AnalyzeBreak(const TreeNode& stmt, const std::unordered_set<EToken>& inSet);
	bool AnalyzeContinue(const TreeNode& stmt, const std::unordered_set<EToken>& inSet);
	bool AnalyzeFn(const TreeNode& stmt);
	bool AnalyzeReturn(const TreeNode& stmt, const std::unordered_set<EToken>& inSet);
	bool AnalyzeCompound(const TreeNode& stmt, const std::unordered_set<EToken>& inSet);
	bool AnalyzeClass(const TreeNode& stmt);

	bool AnalyzeInvokeExp(const TreeNode& stmt);
	bool AnalyzeExp(const TreeNode& stmt);
};

}
