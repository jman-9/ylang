#include "SemanticAnalyzer.h"
#include "BuiltinFuncTable.h"
#include <format>
using namespace std;


namespace ycom
{

static const BuiltinFuncTable _builtinFuncTbl;


SemanticAnalyzer::SemanticAnalyzer()
{
	_symTbl.resize(1);
}

SemanticAnalyzer::~SemanticAnalyzer()
{
}

bool SemanticAnalyzer::Analyze(const TreeNode& code)
{
	for(const auto& stmt : code.childs)
	{
		if(!AnalyzeStmt(*stmt, {}))
			return false;
	}

	return true;
}

void SemanticAnalyzer::OpenScope(ScopeManager::ScopeType type)
{
	switch(type)
	{
	case ScopeManager::SCOPE_LOCAL:	_scopeMgr.AddLocalScope(); break;
	case ScopeManager::SCOPE_GLOBAL:_scopeMgr.AddGlobalScope(); break;
	case ScopeManager::SCOPE_CLASS:	_scopeMgr.AddClassScope(); break;
	}

	_symTbl.resize(_symTbl.size() + 1);
	_symTbl.back() = _symTbl[_symTbl.size() - 2];
}
void SemanticAnalyzer::CloseScope()
{
	_symTbl.pop_back();
	_scopeMgr.PopScope();
}
void SemanticAnalyzer::OpenCompound()
{
	OpenScope(ScopeManager::SCOPE_LOCAL);
}
void SemanticAnalyzer::CloseCompound()
{
	CloseScope();
}

bool SemanticAnalyzer::AnalyzeStmt(const TreeNode& stmt, const unordered_set<EToken>& inSet)
{
	switch(stmt.self.kind)
	{
	case EToken::Include : return AnalyzeInclude(stmt);
	case EToken::If : return AnalyzeIf(stmt, inSet);
	case EToken::For : return AnalyzeFor(stmt, inSet);
	case EToken::Break : return AnalyzeBreak(stmt, inSet);
	case EToken::Continue : return AnalyzeContinue(stmt, inSet);
	case EToken::Fn : return AnalyzeFn(stmt);
	case EToken::Return : return AnalyzeReturn(stmt, inSet);
	case EToken::LBrace : return AnalyzeCompound(stmt, inSet);
	case EToken::Class: return AnalyzeClass(stmt);
	default: ;
	}
	return AnalyzeExp(stmt);
}

bool SemanticAnalyzer::AnalyzeExp(const TreeNode& stmt)
{
	if(stmt.self == EToken::Assign)
	{
		bool rhsOk = AnalyzeExp(*stmt.childs.back());
		if(!rhsOk)
		{//todo trace
			//_errors.push_back(ErrorBuilder::Default(stmt.self.line, "assignment
			//throw 'n';
			return false;
		}

		// TODO
		// LValue 체크? 구문분석에서?
		// postfix LValue 체크 필요
		auto& lhs = stmt.childs.front();
		auto& lhsTok = lhs->self;
		if(lhsTok == EToken::LValueIndex)
		{//TODO
			if(!AnalyzeExp(*lhs->childs.front()))
				return false;
			if(!AnalyzeExp(*lhs->childs.back()))
				return false;
		}
		else if(lhsTok == EToken::LValueField)
		{
			if(!AnalyzeExp(*lhs->childs.front()))
				return false;
		}
		else
		{
			auto found = _symTbl.back().find(lhsTok.val);
			if(found == _symTbl.back().end() || found->second.preRegister)
			{
				Symbol sym;
				sym.name = lhsTok.val;
				sym.kind = ESymbol::Var;
				_symTbl.back()[ sym.name ] = sym;
			}
		}
		return true;
	}

	if(stmt.self == EToken::Invoke)
	{//TODO 괄호 이용할 경우 처리
		auto& name = stmt.childs[0];
		if(name->self == EToken::Id)
		{
			auto builtinFuncId = _builtinFuncTbl.GetFuncId(name->self.val);
			if(!builtinFuncId)
			{
				auto found = _symTbl.back().find(name->self.val);
				if(found == _symTbl.back().end())
				{
					_errors.push_back(ErrorBuilder::NotFound(name->self.line, name->self.val));
					return false;
				}

				if(found->second.kind == ESymbol::Fn)
				{
					//TODO 가변인자
					if(stmt.childs.size() - 1 != found->second.params.size())
					{//todo message
						_errors.push_back(ErrorBuilder::Default(stmt.self.line, format("'{}': no matched arguments", name->self.val)));
						return false;
					}
				}
			}
		}
		else
		{
			if(!AnalyzeExp(*stmt.childs[0]))
			{//todo trace
				return false;
			}
		}

		for(size_t i=1; i<stmt.childs.size(); i++)
		{
			if(!AnalyzeExp(*stmt.childs[i]))
			{//todo trace
				return false;
			}
		}
		return true;
	}

	if(stmt.self == EToken::Id || stmt.self == EToken::Str)
	{
		if((!_nsCtx.IsEmpty() || stmt.self == EToken::Id) && _nsTracker.IsExistingIfAppend(_nsCtx, stmt.self.val))
		{
			_nsCtx.Append(stmt.self.val);
		}
		else
		{
			if(_nsTracker.IsTerminal(_nsCtx))
			{//TODO module member field check
				_nsCtx.Clear();
			}
			else
			{
				_errors.push_back(ErrorBuilder::NotFound(stmt.self.line, stmt.self.val));
				return false;
			}

			if(stmt.self == EToken::Id)
			{
				auto found = _symTbl.back().find(stmt.self.val);
				if(found == _symTbl.back().end())
				{//todo message
					_errors.push_back(ErrorBuilder::NotInitialized(stmt.self.line, stmt.self.val));
					return false;
				}
			}
		}
		return true;
	}

	if(!_nsTracker.IsTerminal(_nsCtx))
	{
		_errors.push_back(ErrorBuilder::NotFound(stmt.self.line, stmt.self.val));
		_nsCtx.Clear();
		return false;
	}

	if(stmt.self.IsIncDecOp())
	{
		if(!CanBeLValue(*stmt.childs.front()))
		{
			_errors.push_back(ErrorBuilder::LValueError(stmt.self.line, stmt.self.val));
			return false;
		}
	}

	for(auto& c : stmt.childs)
	{
		if(!AnalyzeExp(*c))
		{//todo trace
			return false;
		}
	}

	return true;
}

bool SemanticAnalyzer::AnalyzeInclude(const TreeNode& stmt)
{
	if(stmt.childs.size() != 1)
	{//todo
		return false;
	}

	auto& modPath = stmt.childs.front()->self;
	if(modPath != EToken::Str)
	{//todo clarify
		_errors.push_back(ErrorBuilder::Default(modPath.line, "TODO : " + modPath.val));
		return false;
	}

	auto res = NamespaceUtil::ResolveInclude(modPath.val);

	auto found = _symTbl.back().find(res.namespacePath);
	if(found != _symTbl.back().end())
	{
		_errors.push_back(ErrorBuilder::AlreadyExists(modPath.line, modPath.val));
		return false;
	}

	_nsTracker.AddTrackingPath(res.namespacePath);
	_symTbl.back()[res.namespacePath] = Symbol{ .name = res.namespacePath, .kind = ESymbol::Mod };
	return true;
}

bool SemanticAnalyzer::AnalyzeFor(const TreeNode& stmt, const unordered_set<EToken>& inSet)
{
	auto& init = *stmt.childs[0];
	auto& cond = *stmt.childs[1];
	auto& update = *stmt.childs[2];
	auto& block = *stmt.childs[3];
	if(!AnalyzeExp(init))
	{//todo trace
		return false;
	}
	if(!AnalyzeExp(cond))
	{//todo trace
		return false;
	}
	if(!AnalyzeExp(update))
	{//todo trace
		return false;
	}

	auto inSetLocal = inSet;
	inSetLocal.insert(EToken::For);
	if(!AnalyzeStmt(block, inSetLocal))
	{//todo trace
		return false;
	}
	return true;
}

bool SemanticAnalyzer::AnalyzeBreak(const TreeNode& stmt, const unordered_set<EToken>& inSet)
{
	if(inSet.contains(EToken::For))
		return true;
	_errors.push_back(ErrorBuilder::Default(stmt.self.line, "break statement not within a loop"));
	return false;
}

bool SemanticAnalyzer::AnalyzeContinue(const TreeNode& stmt, const unordered_set<EToken>& inSet)
{
	if(inSet.contains(EToken::For))
		return true;
	_errors.push_back(ErrorBuilder::Default(stmt.self.line, "continue statement not within a loop"));
	return false;
}

bool SemanticAnalyzer::AnalyzeIf(const TreeNode& stmt, const unordered_set<EToken>& inSet)
{
	auto& test = *stmt.childs[0];
	auto& _true = *stmt.childs[1];

	if(!AnalyzeExp(test))
	{//todo trace
		return false;
	}
	if(!AnalyzeStmt(_true, inSet))
	{//todo trace
		return false;
	}

	if(stmt.childs.size() > 2)
	{
		auto& _false = *stmt.childs[2];
		if(!AnalyzeStmt(_false, inSet))
		{
			//todo trace
			return false;
		}
	}

	return true;
}

bool SemanticAnalyzer::AnalyzeFn(const TreeNode& stmt)
{
	if(_scopeMgr.GetCurScope() == ScopeManager::SCOPE_LOCAL)
	{//TODO
		_errors.push_back(ErrorBuilder::Default(stmt.self.line, "nested function: currently not supported"));
		return false;
	}


	auto& name = stmt.self.val;
	auto& params = stmt.childs[0]->childs;
	auto& block = *stmt.childs[1];

	auto found = _symTbl.back().find(name);
	//TODO ctor redundancy
	if(found != _symTbl.back().end() && !found->second.preRegister && found->second.kind != ESymbol::Cls)
	{
		//todo message
		_errors.push_back(ErrorBuilder::AlreadyExists(stmt.self.line, stmt.self.val));
		return false;
	}

	Symbol sym;
	sym.name = name;
	sym.kind = ESymbol::Fn;
	for(auto& p : params)
	{
		auto found = _symTbl.back().find(p->self.val);
		if(found != _symTbl.back().end())
		{
			//todo message
			_errors.push_back(ErrorBuilder::AlreadyExists(stmt.self.line, p->self.val));
			return false;
		}

		Param prm;
		prm.name = p->self.val;
		sym.params.push_back(prm);
	}
	_symTbl.back()[ name ] = sym;


	OpenScope(ScopeManager::SCOPE_LOCAL);

	for(auto& v : sym.params)
	{
		Symbol sym;
		sym.name = v.name;
		sym.kind = ESymbol::Var;
		_symTbl.back()[ v.name ] = sym;
	}

	if(!AnalyzeStmt(block, { EToken::Fn }))
	{
		//todo trace
		_symTbl.back().erase(name);
		return false;
	}

	CloseScope();
	return true;
}

bool SemanticAnalyzer::AnalyzeReturn(const TreeNode& stmt, const unordered_set<EToken>& inSet)
{
	if(!inSet.contains(EToken::Fn))
	{
		_errors.push_back(ErrorBuilder::Default(stmt.self.line, "return statement not within a function"));
		return false;
	}

	for(auto& c : stmt.childs)
	{
		if(!AnalyzeExp(*c))
		{//todo trace
			return false;
		}
	}
	return true;
}

bool SemanticAnalyzer::AnalyzeCompound(const TreeNode& stmt, const unordered_set<EToken>& inSet)
{
	if(stmt.self != EToken::LBrace)
		throw 'n';

	OpenCompound();

	for(auto& itm : stmt.childs)
	{
		if(!AnalyzeStmt(*itm, inSet))
			return false;
	}

	CloseCompound();
	return true;
}

bool SemanticAnalyzer::AnalyzeClass(const TreeNode& stmt)
{
	if(stmt.self != EToken::Class)
		throw 'n';

	if(_scopeMgr.GetCurScope() == ScopeManager::SCOPE_LOCAL)
	{//TODO
		_errors.push_back(ErrorBuilder::Default(stmt.self.line, "nested class: currently not supported"));
		return false;
	}

	for(auto& itm : stmt.childs)
	{
		if(itm->self != EToken::Assign && itm->self != EToken::Fn)
		{
			_errors.push_back(ErrorBuilder::Default(stmt.self.line, format("'{}': not allowed in class statement", stmt.self.val)));
			return false;
		}
	}

	auto& name = stmt.self.val;

	auto found = _symTbl.back().find(name);
	if(found != _symTbl.back().end())
	{
		_errors.push_back(ErrorBuilder::AlreadyExists(stmt.self.line, stmt.self.val));
		return false;
	}

	Symbol sym;
	sym.name = name;
	sym.kind = ESymbol::Cls;
	_symTbl.back()[ name ] = sym;

	OpenScope(ScopeManager::SCOPE_CLASS);

	for(auto& itm : stmt.childs)
	{
		if(itm->self != EToken::Assign) continue;

		const auto& id = itm->childs.front()->self;
		auto found = _symTbl.back().find(id.val);
		if(found == _symTbl.back().end())
		{
			Symbol sym;
			sym.preRegister = true;
			sym.name = id.val;
			sym.kind = ESymbol::Var;
			_symTbl.back()[ sym.name ] = sym;
		}
		else if(!found->second.preRegister)
		{
			_errors.push_back(ErrorBuilder::AlreadyExists(id.line, id.val));
			return false;
		}

		if(!AnalyzeExp(*itm)) return false;
	}

	for(auto& itm : stmt.childs)
	{
		if(itm->self != EToken::Fn) continue;

		const auto& fn = itm->self;
		auto found = _symTbl.back().find(fn.val);
		if(found == _symTbl.back().end())
		{
			Symbol sym;
			sym.preRegister = true;
			sym.name = fn.val;
			sym.kind = ESymbol::Fn;
			for(auto& p : itm->childs[0]->childs)
			{
				Param prm;
				prm.name = p->self.val;
				sym.params.push_back(prm);
			}
			_symTbl.back()[ sym.name ] = sym;
		}
		else if(!found->second.preRegister && found->second.kind != ESymbol::Cls)
		{
			_errors.push_back(ErrorBuilder::AlreadyExists(fn.line, fn.val));
			return false;
		}
	}

	for(auto& itm : stmt.childs)
	{
		if(itm->self != EToken::Fn) continue;
		if(!AnalyzeFn(*itm)) return false;
	}

	CloseScope();
	return true;
}

bool SemanticAnalyzer::CanBeLValue(const TreeNode& stmt)
{
	const TreeNode* cur = &stmt;
	for( ; cur; cur = cur->childs.front().get())
	{
		auto& curTok = cur->self;
		if(curTok == EToken::LParen)
		{
			continue;
		}

		if(curTok == EToken::LValueField) break;

		if(curTok != EToken::Id && curTok != EToken::Dot && curTok != EToken::Index && curTok != EToken::LValueIndex)
		{
			return false;
		}
		if(curTok == EToken::Id)
		{
			auto found = _symTbl.back().find(curTok.val);
			if(found != _symTbl.back().end())
			{
				if(found->second.kind != ESymbol::Var && found->second.kind != ESymbol::Field)
					return false;
			}
		}

		if(cur->childs.empty()) break;
	}

	return true;
}

}
