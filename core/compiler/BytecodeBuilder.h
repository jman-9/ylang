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

	int AddOrNot(const Constant& con);
	int GetIdx(const Constant& con) const;

	Constant TokenToConstant(const Token& tok) const;

	struct ConstHash {
		std::size_t operator()(const Constant& c) const {
			std::string val;
			switch(c._type) {
			case Constant::INT: val = std::to_string(c._int); break;
			case Constant::FLOAT: val = std::to_string(c._float); break;
			case Constant::STR: val = c._str; break;
			case Constant::CLOSURE: val = c._closure._uniqueName; break;
			case Constant::GLOBAL_FN: val = c._str; break;
			default: throw std::logic_error("not implemented");
			}
			return std::hash<std::string>()(val) ^ (static_cast<std::size_t>(c._type) << 1);
		}
	};

	struct ConstEqual {
		bool operator()(const Constant& a, const Constant& b) const {
			if(a._type != b._type) return false;

			switch(a._type) {
			case Constant::INT: return a._int == b._int;
			case Constant::FLOAT: return a._float == b._float;
			case Constant::STR: return a._str == b._str;
			case Constant::CLOSURE: return a._closure._uniqueName == b._closure._uniqueName;
			case Constant::GLOBAL_FN: return a._str == b._str;
			default: throw std::logic_error("not implemented");
			}
		}
	};

	std::unordered_map<Constant, uint16_t, ConstHash, ConstEqual> _constMap;
};


class BytecodeBuilder
{
public:
	BytecodeBuilder();
	~BytecodeBuilder();

	bool Build(const TreeNode& code, Program& retProgram, const std::unordered_map<std::string, Program>* programTable = nullptr, const std::vector<std::string>& paths = {});

protected:
	uint32_t _reg;
	Program _prg;
	ConstTable _constTbl;
	ScopeManager _scopeMgr;
	std::vector<std::string> _paths;

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
	bool BuildCompound(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildReturn(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildContinue(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildBreak(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildClass(Bytecode& retCtx, const TreeNode& stmt);

	bool BuildInvokeExp(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildListExp(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildDictExp(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildBytesExp(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildIndexExp(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildLValueFieldExp(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildTernaryExp(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildExp(Bytecode& retCtx, const TreeNode& stmt, bool root);

	bool BuildFnReal(Bytecode& retCtx, const TreeNode& stmt, ESymbol fnType = ESymbol::Fn);
	bool BuildClosure(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildFn(Bytecode& retCtx, const TreeNode& stmt);

	void DetectCaptures(std::unordered_map<std::string, ScopeManager::SymbolData>& retCaptures, const TreeNode& stmt) const;
};

}
