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

		bool IsNone() const { return kind == NONE; }
	};

	struct SymbolKey
	{
		std::string name;
	};

	struct SymbolData
	{
		Symbol sym;
		Idx idx;
	};

public:
	ScopeManager();
	~ScopeManager();

	void AddGlobalScope();
	void AddLocalScope();
	void AddClassScope();
	void PopScope();

	ScopeType GetCurScope() const;
	bool IsUnderClassScope() const;

	Idx AddOrNot(const Symbol& sym);
	Idx AddOrReplace(const Symbol& sym);
	Idx AddForce(const Symbol& sym);
	SymbolData Erase(const std::string& name);
	Idx GetIdx(const std::string& name) const;
	Symbol GetSymbol(const std::string& name) const;

protected:
	struct SymKeyHash {
		std::size_t operator()(const SymbolKey& t) const noexcept {
			return std::hash<std::string>()(t.name);
		}
	};

	struct SymKeyEqual {
		bool operator()(const SymbolKey& a, const SymbolKey& b) const noexcept {
			return a.name == b.name;
		}
	};

	struct Scope
	{
		ScopeType type;
		int startLocalIdx;
		std::unordered_map<SymbolKey, SymbolData, SymKeyHash, SymKeyEqual> symTbl;
	};

	std::vector<Scope> _scopeTbl;

	int _localIdxOffset;

	SymbolData GetSymbolData(const std::string& name) const;
	const SymbolData* GetSymbolDataRef(const std::string& name) const;
};

}
