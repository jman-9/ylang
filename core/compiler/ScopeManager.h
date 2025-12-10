#pragma once
#include "Symbol.h"
#include <string>
#include <vector>
#include <unordered_map>


namespace ycom
{

class ScopeManager
{
	friend class BytecodeBuilder;
public:
	enum ScopeType
	{
		SCOPE_GLOBAL,
		SCOPE_LOCAL,
		SCOPE_CLASS,
	};

	struct Idx
	{
		enum Kind
		{
			NONE,

			GLOBAL,
			LOCAL,
			FIELD,
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
	ScopeManager();
	~ScopeManager();

	void AddGlobalScope();
	void AddLocalScope();
	void AddClassScope();
	void PopScope();

	ScopeType GetCurScope() const;

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

	struct Scope
	{
		ScopeType type;
		int startLocalIdx;
		std::unordered_map<Symbol, Idx, SymbolHash, SymbolEqual> symTbl;
	};

	std::vector<Scope> _scopeTbl;

	int _localIdxOffset;

	SymbolData GetSymbolData(const std::string& name) const;
};

}
