#pragma once
#include "TreeNode.h"
#include "ScopeManager.h"
#include "Bytecode.h"
#include "Program.h"
#include "NamespaceUtil.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <stack>


namespace ycom
{

class ConstTable
{
	friend class BytecodeBuilder;

	int AddOrNot(const Token& tok);
	int GetIdx(const Token& tok) const;

	struct TokenHash {
		std::size_t operator()(const Token& t) const noexcept {
			return std::hash<std::string>()(t.val) ^ (static_cast<std::size_t>(t.kind) << 1);
		}
	};

	struct TokenEqual {
		bool operator()(const Token& a, const Token& b) const noexcept {
			return a.kind == b.kind && a.val == b.val;
		}
	};

	std::unordered_map<Token, uint16_t, TokenHash, TokenEqual> _constMap;
};


class BytecodeBuilder
{
public:
	BytecodeBuilder();
	~BytecodeBuilder();

	bool Build(const TreeNode& code, Program& retProgram, const std::unordered_map<std::string, Program>* programTable = nullptr);

protected:
	uint32_t _reg;
	Program _prg;
	ConstTable _constTbl;
	ScopeManager _scopeMgr;

	const std::unordered_map<std::string, Program>* _prgTbl = nullptr;

	struct LoopControl
	{
		std::vector<int> contLines;
		std::vector<int> breakLines;
		int pushSpCnt = 0;
	};
	std::stack<LoopControl> _loopStack;

	struct FnControl
	{
		std::vector<int> retLines;
		int pushSpCnt = 0;
	};
	std::stack<FnControl> _fnStack;

	std::stack<Class*> _clsStack;

	NamespaceUtil::Context _nsCtx;
	NamespaceUtil::Tracker _nsTracker;
	ScopeManager::Idx GetNamespacePathIdx() const;

	void BuildBlockOpen(Bytecode& retCtx);
	void BuildBlockClose(Bytecode& retCtx);

	bool BuildStmt(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildInclude(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildFor(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildIf(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildFn(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildCompound(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildReturn(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildContinue(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildBreak(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildClass(Bytecode& retCtx, const TreeNode& stmt);

	bool BuildInvokeExp(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildListExp(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildDictExp(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildIndexExp(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildLValueFieldExp(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildExp(Bytecode& retCtx, const TreeNode& stmt, bool root);
};

}
