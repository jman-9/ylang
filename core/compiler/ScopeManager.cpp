#include "ScopeManager.h"
#include <stdexcept>
using namespace std;

namespace ycom
{

ScopeManager::ScopeManager()
{
	_scopeTbl.push_back({});
	_scopeTbl.back().type = SCOPE_GLOBAL;
}
ScopeManager::~ScopeManager()
{
}

void ScopeManager::AddLocalScope()
{
	_scopeTbl.back()._localScopes.push_back({});
}
void ScopeManager::AddClassScope()
{
	_scopeTbl.push_back({});
	_scopeTbl.back().type = SCOPE_CLASS;
}
void ScopeManager::AddClosureScope()
{
	_scopeTbl.push_back({});
	_scopeTbl.back().type = SCOPE_CLOSURE;
}
void ScopeManager::PopScope()
{
	if(!_scopeTbl.back()._localScopes.empty())
	{
		_scopeTbl.back()._localScopes.pop_back();
		return;
	}

	if(_scopeTbl.size() == 1)
		throw logic_error("unable to pop global scope");

	_scopeTbl.pop_back();
}

ScopeManager::ScopeType ScopeManager::GetCurScope() const
{
	if(!_scopeTbl.back()._localScopes.empty())
		return SCOPE_LOCAL;
	return _scopeTbl.back().type;
}
bool ScopeManager::IsUnderGlobalScope() const
{
	return _scopeTbl.back().type == SCOPE_GLOBAL;
}
bool ScopeManager::IsUnderClassScope() const
{
	return _scopeTbl.back().type == SCOPE_CLASS;
}
bool ScopeManager::IsUnderClosureScope() const
{
	return _scopeTbl.back().type == SCOPE_CLOSURE;
}

ScopeManager::Idx ScopeManager::AddOrNot(const Symbol& sym)
{
	auto idx = GetIdx(sym.name);
	if(idx.kind != Idx::NONE)
	{
		return idx;
	}
	return AddForce(sym);
}
ScopeManager::Idx ScopeManager::AddForce(const Symbol& sym)
{
	Idx idx;
	auto scope = GetCurScope();
	switch(scope)
	{
	case SCOPE_GLOBAL:
		idx.kind = Idx::GLOBAL;
		idx.idx = GetGlobalSymbolTable().size();
		break;

	case SCOPE_LOCAL:
		idx.kind = Idx::LOCAL;
		idx.idx = GetLastLocalIndex();
		break;

	case SCOPE_CLASS:
		if(sym.kind == ESymbol::Fn)
		{
			idx.kind = Idx::LOCAL;
			idx.idx = 0;
		}
		else if(sym.kind == ESymbol::Field)
		{
			idx.idx = _scopeTbl.back()._symTbl.size();
			idx.kind = Idx::FIELD;
		}
		else
			return {};
		break;
	}

	SymbolKey sk = { sym.name };
	if(_scopeTbl.back()._localScopes.empty())
		_scopeTbl.back()._symTbl[sk] = SymbolData{ sym, idx };
	else
		_scopeTbl.back()._localScopes.back()[sk] = SymbolData{ sym, idx };

	//TODO nested func proc
	return idx;
}
ScopeManager::Idx ScopeManager::AddOrReplace(const Symbol& sym)
{
	auto r = (SymbolData*)GetSymbolDataRef(sym.name);
	if(!r) return AddForce(sym);
	r->sym = sym;
	return r->idx;
}

ScopeManager::SymbolData ScopeManager::Erase(const string& name)
{
	for(auto& localTbl : _scopeTbl.back()._localScopes)
	{
		auto found = localTbl.find( { .name = name } );
		if(found != localTbl.end())
		{
			SymbolData t = found->second;
			localTbl.erase(found);
			return t;
		}
	}

	auto found = _scopeTbl.back()._symTbl.find( { .name = name } );
	if(found != _scopeTbl.back()._symTbl.end())
	{
		SymbolData t = found->second;
		_scopeTbl.back()._symTbl.erase(found);
		return t;
	}
	return {};
}

ScopeManager::Idx ScopeManager::GetIdx(const string& name) const
{
	return GetSymbolData(name).idx;
}

Symbol ScopeManager::GetSymbol(const string& name) const
{
	return GetSymbolData(name).sym;
}

ycom::ScopeManager::SymbolMap& ScopeManager::GetGlobalSymbolTable()
{
	return _scopeTbl.front()._symTbl;
}
const ycom::ScopeManager::SymbolMap& ScopeManager::GetGlobalSymbolTable() const
{
	return _scopeTbl.front()._symTbl;
}

ScopeManager::SymbolData ScopeManager::GetSymbolData(const string& name) const
{
	auto r = GetSymbolDataRef(name);
	return r ? *r : SymbolData{};
}

const ScopeManager::SymbolData* ScopeManager::GetSymbolDataRef(const string& name) const
{
	auto found = GetGlobalSymbolTable().find( { .name = name } );
	if(found != GetGlobalSymbolTable().end())
	{
		return &found->second;
	}

	for(auto& localTbl : _scopeTbl.back()._localScopes)
	{
		auto found = localTbl.find( { .name = name } );
		if(found != localTbl.end())
		{
			return &found->second;
		}
	}

	if(&GetGlobalSymbolTable() == &_scopeTbl.back()._symTbl)
	{
		return nullptr;
	}
	else
	{
		auto found = _scopeTbl.back()._symTbl.find( { .name = name } );
		if(found != _scopeTbl.back()._symTbl.end())
		{
			return &found->second;
		}
		return nullptr;
	}
}

int ScopeManager::GetLastLocalIndex() const
{
	int last = 0;
	for(auto& localTbl : _scopeTbl.back()._localScopes)
	{
		last += localTbl.size();
	}
	return last;
}

}
