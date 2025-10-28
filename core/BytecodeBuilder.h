#pragma once
#include "TreeNode.h"
#include "Symbol.h"
#include "Instruction.h"
#include "Bytecode.h"
#include <string>
#include <vector>
#include <map>
#include <unordered_map>


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
	struct Idx
	{
		enum Kind
		{
			NONE,

			GLOBAL,
			LOCAL,
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

	void AddScope();
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

	SymbolData GetSymbolData(const std::string& name) const;

	int GetNewSlotIdx() const;
	int GetGlobalSymbolCnt() const;
	int GetLocalSymbolCnt() const;
	int GetSymbolCnt() const;
};



class BytecodeBuilder
{
public:
	BytecodeBuilder(const TreeNode& code);
	~BytecodeBuilder();

	bool Build(Bytecode& retCode);

protected:
	const TreeNode& _code;
	uint32_t _reg;
	std::vector<Instruction> _bytecode;
	ConstTable _constTbl;
	SymbolTable _symTbl;

	//std::vector<std::map<std::string, Symbol>> _symTbl;

	bool BuildStmt(const TreeNode& stmt);
	bool BuildFor(const TreeNode& stmt);
	bool BuildIf(const TreeNode& stmt);
	bool BuildFn(const TreeNode& stmt);
	bool BuildCompound(const TreeNode& stmt);
	bool BuildExp(const TreeNode& stmt, bool root);
	bool BuildInvoke(const TreeNode& stmt);
};
