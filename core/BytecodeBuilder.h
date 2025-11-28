#pragma once
#include "TreeNode.h"
#include "Symbol.h"
#include "Instruction.h"
#include "Bytecode.h"
#include "Program.h"
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <stack>


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

class SymbolTable
{
public:
	enum ScopeType
	{
		SCOPE_BLOCK,
		SCOPE_FUNC,
	};

	struct Idx
	{
		enum Kind
		{
			NONE,

			GLOBAL,
			LOCAL,
			MEMBER,
		};
		Kind kind = NONE;
		int idx = 0;
	};

	struct SymbolData
	{
		Idx idx;
		Symbol sym;
	};

public:
	SymbolTable();
	~SymbolTable();

	void AddBlockScope();
	void AddFuncScope();
	void PopScope();

	Idx AddOrNot(const Symbol& sym);
	Idx GetIdx(const std::string& name) const;
	Symbol GetSymbol(const std::string& name) const;

protected:
	struct SymbolHash {
		std::size_t operator()(const Symbol& t) const noexcept {
			return std::hash<std::string>()(t.name);
		}
	};

	struct SymbolEqual {
		bool operator()(const Symbol& a, const Symbol& b) const noexcept {
			return a.name == b.name;
		}
	};

	std::vector<std::unordered_map<Symbol, int, SymbolHash, SymbolEqual>> _symTbl;
	std::vector<ScopeType> _scopeTbl;

	SymbolData GetSymbolData(const std::string& name) const;

	int GetNewSlotIdx() const;
	int GetGlobalSymbolCnt() const;
	int GetLocalSymbolCnt() const;
	int GetSymbolCnt() const;
	int GetBehindFuncScopeCnt(int idx) const;
};



class BytecodeBuilder
{
public:
	BytecodeBuilder();
	~BytecodeBuilder();

	bool Build(const TreeNode& code, Program& retProgram);

protected:
	uint32_t _reg;
	Program _prg;
	ConstTable _constTbl;
	SymbolTable _symTbl;

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



	void BuildBlockOpen(Bytecode& retCtx);
	void BuildBlockClose(Bytecode& retCtx);

	bool BuildStmt(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildInclude(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildFor(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildIf(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildFn(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildCompound(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildExp(Bytecode& retCtx, const TreeNode& stmt, bool root);
	bool BuildReturn(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildContinue(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildBreak(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildList(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildDict(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildIndex(Bytecode& retCtx, const TreeNode& stmt);
	bool BuildStruct(Bytecode& retCtx, const TreeNode& stmt);

	//inline int endOfCode() const;
	//inline int nextCodeSlot() const;

private:
/*	template<EOpcode Op>
	void FillBytecode(int ln);
	template<class OpType>
	void FillBytecode(int ln, const OpType& inst, const TreeNode* stmt = nullptr);
	template<class OpType>
	void FillBytecode(int ln, const OpType& inst, const TreeNode& stmt);
	template<EOpcode Op>
	int PushBytecode();
	template<class OpType>
	int PushBytecode(const OpType& inst, const TreeNode* stmt = nullptr);
	template<class OpType>
	int PushBytecode(const OpType& inst, const TreeNode& stmt);*/
};
