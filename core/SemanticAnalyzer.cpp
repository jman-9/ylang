#include "SemanticAnalyzer.h"


SemanticAnalyzer::SemanticAnalyzer(const TreeNode& code)
	: _code(code)
{
	_symTbl.resize(1);
}

SemanticAnalyzer::~SemanticAnalyzer()
{
}

bool SemanticAnalyzer::Analyze()
{
	for(const auto& stmt : _code.childs)
	{
		AnalyzeStmt(*stmt);
	}

	return true;
}

bool SemanticAnalyzer::AnalyzeStmt(const TreeNode& stmt)
{
	switch(stmt.self.kind)
	{
	case EToken::For : return AnalyzeFor(stmt);
	case EToken::If : return AnalyzeIf(stmt);
	case EToken::Fn : return AnalyzeFn(stmt);
	case EToken::LBrace : return AnalyzeCompound(stmt);
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
		{
			throw 'n';
		}

		// TODO
		// LValue 체크? 구문분석에서?
		// postfix LValue 체크 필요
		auto found = _symTbl.back().find(stmt.childs.front()->self.val);
		if(found == _symTbl.back().end())
		{
			Symbol sym;
			sym.name = stmt.childs.front()->self.val;
			sym.kind = ESymbol::Var;
			_symTbl.back()[ sym.name ] = sym;
		}
		return true;
	}

	if(stmt.self == EToken::Invoke)
	{//TODO 괄호 이용할 경우 처리
		auto& name = stmt.childs[0];
		if(name->self == EToken::Id)
		{
			if(name->self.val == "print" || name->self.val == "println")
			{//TODO builtin table
				if(stmt.childs.size() - 1 != 1)
				{
					throw 'n';
				}
			}
			else
			{
				auto found = _symTbl.back().find(name->self.val);
				if(found == _symTbl.back().end())
				{
					throw 'n';
				}

				//TODO 가변인자
				if(stmt.childs.size() - 1 != found->second.params.size())
				{
					throw 'n';
				}
			}
		}

		return true;
	}

	if(stmt.self == EToken::Id)
	{
		auto found = _symTbl.back().find(stmt.self.val);
		if(found == _symTbl.back().end())
		{
			throw 'n';
		}

		return true;
	}

	for(auto& c : stmt.childs)
	{
		if(!AnalyzeExp(*c))
		{
			throw 'n';
		}
	}

	return true;
}

bool SemanticAnalyzer::AnalyzeFor(const TreeNode& stmt)
{
	if(stmt.self != EToken::For)
		throw 'n';

	auto& init = *stmt.childs[0];
	auto& cond = *stmt.childs[1];
	auto& update = *stmt.childs[2];
	auto& block = *stmt.childs[3];
	if(!AnalyzeExp(init))
	{
		throw 'n';
	}
	if(!AnalyzeExp(cond))
	{
		throw 'n';
	}
	if(!AnalyzeExp(update))
	{
		throw 'n';
	}
	if(!AnalyzeStmt(block))
	{
		throw 'n';
	}

	return true;
}

bool SemanticAnalyzer::AnalyzeIf(const TreeNode& stmt)
{
	if(stmt.self != EToken::If)
		throw 'n';

	auto& test = *stmt.childs[0];
	auto& _true = *stmt.childs[1];

	if(!AnalyzeExp(test))
	{
		throw 'n';
	}
	if(!AnalyzeStmt(_true))
	{
		throw 'n';
	}

	if(stmt.childs.size() > 2)
	{
		auto& _false = *stmt.childs[2];
		if(!AnalyzeStmt(_false))
		{
			throw 'n';
		}
	}

	return true;
}

bool SemanticAnalyzer::AnalyzeFn(const TreeNode& stmt)
{
	if(stmt.self != EToken::Fn)
		throw 'n';

	auto& name = stmt.self.val;
	auto& params = stmt.childs[0]->childs;
	auto& block = *stmt.childs[1];

	auto found = _symTbl.back().find(name);
	if(found != _symTbl.back().end())
	{
		throw 'n';
	}

	Symbol sym;
	sym.name = name;
	sym.kind = ESymbol::Fn;
	for(auto& p : params)
	{
		auto found = _symTbl.back().find(p->self.val);
		if(found != _symTbl.back().end())
		{
			throw 'n';
		}

		Param prm;
		prm.name = p->self.val;
		sym.params.push_back(prm);
	}

	if(block.self == EToken::LBrace)
	{
		if(!AnalyzeCompound(block, sym.params))
		{
			throw 'n';
		}
	}
	else if(!AnalyzeStmt(block))
	{
		throw 'n';
	}

	_symTbl.back()[ name ] = sym;
	return true;
}

bool SemanticAnalyzer::AnalyzeCompound(const TreeNode& stmt, const std::vector<Param>& stackVars /* = std::vector<Param>() */)
{
	if(stmt.self != EToken::LBrace)
		throw 'n';

	_symTbl.resize(_symTbl.size() + 1);
	_symTbl.back() = _symTbl[_symTbl.size() - 2];

	for(auto& v : stackVars)
	{
		Symbol sym;
		sym.name = v.name;
		sym.kind = ESymbol::Var;
		_symTbl.back()[ v.name ] = sym;
	}

	for(auto& itm : stmt.childs)
	{
		AnalyzeStmt(*itm);
	}

	_symTbl.pop_back();
	return true;
}
