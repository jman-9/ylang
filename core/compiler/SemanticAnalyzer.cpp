#include "SemanticAnalyzer.h"
#include "BuiltinFuncTable.h"
#include <format>


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
		if(!AnalyzeStmt(*stmt))
			return false;
	}

	return true;
}

void SemanticAnalyzer::OpenCompound()
{
	_scopeMgr.AddLocalScope();
	_symTbl.resize(_symTbl.size() + 1);
	_symTbl.back() = _symTbl[_symTbl.size() - 2];
}
void SemanticAnalyzer::CloseCompound()
{
	_symTbl.pop_back();
	_scopeMgr.PopScope();
}

bool SemanticAnalyzer::AnalyzeStmt(const TreeNode& stmt)
{
	switch(stmt.self.kind)
	{
	case EToken::Include : return AnalyzeInclude(stmt);
	case EToken::For : return AnalyzeFor(stmt);
	case EToken::If : return AnalyzeIf(stmt);
	case EToken::Fn : return AnalyzeFn(stmt);
	case EToken::LBrace : return AnalyzeCompound(stmt);
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
	if(stmt.self != EToken::Include)
		//todo trace
		throw 'n';

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

bool SemanticAnalyzer::AnalyzeFor(const TreeNode& stmt)
{
	if(stmt.self != EToken::For)
		//todo trace
		throw 'n';

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
	if(!AnalyzeStmt(block))
	{//todo trace
		return false;
	}

	return true;
}

bool SemanticAnalyzer::AnalyzeIf(const TreeNode& stmt)
{
	if(stmt.self != EToken::If)
		//todo trace
		throw 'n';

	auto& test = *stmt.childs[0];
	auto& _true = *stmt.childs[1];

	if(!AnalyzeExp(test))
	{//todo trace
		return false;
	}
	if(!AnalyzeStmt(_true))
	{//todo trace
		return false;
	}

	if(stmt.childs.size() > 2)
	{
		auto& _false = *stmt.childs[2];
		if(!AnalyzeStmt(_false))
		{
			//todo trace
			return false;
		}
	}

	return true;
}

bool SemanticAnalyzer::AnalyzeFn(const TreeNode& stmt)
{
	if(stmt.self != EToken::Fn)
		//todo trace
		throw 'n';

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


	OpenCompound();

	for(auto& v : sym.params)
	{
		Symbol sym;
		sym.name = v.name;
		sym.kind = ESymbol::Var;
		_symTbl.back()[ v.name ] = sym;
	}

	if(block.self == EToken::LBrace)
	{
		if(!AnalyzeCompound(block))
		{
			//todo trace
			_symTbl.back().erase(name);
			return false;
		}
	}
	else if(!AnalyzeStmt(block))
	{
		//todo trace
		_symTbl.back().erase(name);
		return false;
	}

	CloseCompound();
	return true;
}

bool SemanticAnalyzer::AnalyzeCompound(const TreeNode& stmt)
{
	if(stmt.self != EToken::LBrace)
		throw 'n';

	OpenCompound();

	for(auto& itm : stmt.childs)
	{
		if(!AnalyzeStmt(*itm))
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
	_scopeMgr.AddClassScope();

	auto& name = stmt.self.val;

	auto found = _symTbl.back().find(name);
	if(found != _symTbl.back().end())
	{
		//todo message
		_errors.push_back(ErrorBuilder::Default(stmt.self.line, format("'{}': already defined", stmt.self.val)));
		return false;
	}

	Symbol sym;
	sym.name = name;
	sym.kind = ESymbol::Cls;
	_symTbl.back()[ name ] = sym;

	_symTbl.resize(_symTbl.size() + 1);
	_symTbl.back() = _symTbl[_symTbl.size() - 2];

	for(auto& itm : stmt.childs)
	{
		switch(itm->self.kind)
		{
		case EToken::Fn:
			{
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
			break;
		case EToken::Assign:
			{
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
			}
			break;

		default:
			throw 'n';//TODO
		}
	}

	for(auto& itm : stmt.childs)
	{
		switch(itm->self.kind)
		{
		case EToken::Fn: if(!AnalyzeFn(*itm)) return false; break;
		case EToken::Assign: if(!AnalyzeExp(*itm)) return false; break;
		}
	}

	_symTbl.pop_back();
	_scopeMgr.PopScope();
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
		if(curTok != EToken::Id && curTok != EToken::Dot && curTok != EToken::Index && curTok != EToken::LValueIndex && curTok != EToken::LValueField)
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
