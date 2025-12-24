#include "ScopeManager.h"
using namespace std;

namespace ycom
{

ScopeManager::ScopeManager()
{
	_localIdxOffset = 0;
	AddGlobalScope();
}
ScopeManager::~ScopeManager()
{
}

void ScopeManager::AddGlobalScope()
{
	Scope scope;
	scope.startLocalIdx = 0;
	scope.type = SCOPE_GLOBAL;
	_scopeTbl.push_back(scope);
}
void ScopeManager::AddLocalScope()
{
	int start = 0;
	if(!_scopeTbl.empty() && _scopeTbl.back().type == SCOPE_LOCAL)
	{
		start = _scopeTbl.back().startLocalIdx + _localIdxOffset;
	}

	Scope scope;
	scope.startLocalIdx = start;
	scope.type = SCOPE_LOCAL;
	_scopeTbl.push_back(scope);
	_localIdxOffset = 0;
}
void ScopeManager::AddClassScope()
{
	Scope scope;
	scope.startLocalIdx = 0;
	scope.type = SCOPE_CLASS;
	_scopeTbl.push_back(scope);
	_localIdxOffset = 0;
}
void ScopeManager::PopScope()
{
	_scopeTbl.pop_back();
	_localIdxOffset = _scopeTbl.back().symTbl.size();
}

ScopeManager::ScopeType ScopeManager::GetCurScope() const
{
	return _scopeTbl.back().type;
}
bool ScopeManager::IsUnderClassScope() const
{
	for(int i=(int)_scopeTbl.size()-1; i>=0; i--)
		if(_scopeTbl[i].type == SCOPE_CLASS) return true;
	return false;
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
	switch(_scopeTbl.back().type)
	{
	case SCOPE_GLOBAL:
		idx.kind = Idx::GLOBAL;
		idx.idx = _scopeTbl.back().symTbl.size();
		break;

	case SCOPE_CLASS:
		if(sym.kind == ESymbol::Fn)
		{
			idx.kind = Idx::LOCAL;
			idx.idx = 0;
		}
		else if(sym.kind == ESymbol::Field)
		{
			idx.idx = _scopeTbl.back().startLocalIdx + _localIdxOffset++;
			idx.kind = Idx::FIELD;
		}
		else
			return {};
		break;

	case SCOPE_LOCAL:
		idx.kind = Idx::LOCAL;
		idx.idx = _scopeTbl.back().startLocalIdx + _localIdxOffset++;
		break;
	}

	SymbolKey sk = { sym.name };
	_scopeTbl.back().symTbl[sk] = SymbolData{ sym, idx };
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
	for(int i=(int)_scopeTbl.size()-1; i>=0; i--)
	{
		auto found = _scopeTbl[i].symTbl.find( { .name = name } );
		if(found != _scopeTbl[i].symTbl.end())
		{
			SymbolData t = found->second;
			_scopeTbl[i].symTbl.erase(found);
			return t;
		}
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

ScopeManager::SymbolData ScopeManager::GetSymbolData(const string& name) const
{
	auto r = GetSymbolDataRef(name);
	return r ? *r : SymbolData{};
}

const ScopeManager::SymbolData* ScopeManager::GetSymbolDataRef(const string& name) const
{
	for(int i=(int)_scopeTbl.size()-1; i>=0; i--)
	{
		auto found = _scopeTbl[i].symTbl.find( { .name = name } );
		if(found != _scopeTbl[i].symTbl.end())
		{
			return &found->second;
		}
	}
	return nullptr;
}

}
