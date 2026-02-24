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
		SCOPE_NONE,
		SCOPE_GLOBAL,
		SCOPE_LOCAL,
		SCOPE_CLASS,
		SCOPE_CLOSURE,
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
		ScopeType scope;
	};

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

	using SymbolMap = std::unordered_map<SymbolKey, SymbolData, SymKeyHash, SymKeyEqual>;

	struct Scope
	{
		ScopeType type;
		SymbolMap _symTbl;
		std::vector<SymbolMap> _localScopes;
	};

public:
	ScopeManager();
	~ScopeManager();

	void AddLocalScope();
	void AddClassScope();
	void AddClosureScope();
	void PopScope();

	ScopeType GetCurScope() const;
	bool IsUnderGlobalScope() const;
	bool IsUnderClassScope() const;
	bool IsUnderClosureScope() const;

	Idx AddOrNot(const Symbol& sym);
	Idx AddOrReplace(const Symbol& sym);
	Idx AddForce(const Symbol& sym);
	SymbolData Erase(const std::string& name);
	Idx GetIdx(const std::string& name) const;
	Symbol GetSymbol(const std::string& name) const;

	SymbolMap& GetGlobalSymbolTable();
	const SymbolMap& GetGlobalSymbolTable() const;

protected:
	std::vector<Scope> _scopeTbl;

	SymbolData GetSymbolData(const std::string& name) const;
	const SymbolData* GetSymbolDataRef(const std::string& name) const;

	int GetLastLocalIndex() const;
};

}
