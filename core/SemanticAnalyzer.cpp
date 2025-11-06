#include "SemanticAnalyzer.h"
#include <format>


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
		{//todo trace
			//_errors.push_back(ErrorBuilder::Default(stmt.self.line, "assignment
			//throw 'n';
			return false;
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
					_errors.push_back(ErrorBuilder::Default(name->self.line, format("'{}': not supported function", name->self.val)));
					return false;
				}
			}
			else
			{
				auto found = _symTbl.back().find(name->self.val);
				if(found == _symTbl.back().end())
				{
					_errors.push_back(ErrorBuilder::Default(name->self.line, format("'{}': function not found", name->self.val)));
					return false;
				}

				//TODO 가변인자
				if(stmt.childs.size() - 1 != found->second.params.size())
				{//todo message
					_errors.push_back(ErrorBuilder::Default(stmt.self.line, format("'{}': no matched arguments", name->self.val)));
					return false;
				}
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

	if(stmt.self == EToken::Id)
	{
		auto found = _symTbl.back().find(stmt.self.val);
		if(found == _symTbl.back().end())
		{//todo message
			_errors.push_back(ErrorBuilder::Default(stmt.self.line, format("'{}': not initialized variable", stmt.self.val)));
			return false;
		}

		return true;
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

	auto& name = stmt.self.val;
	auto& params = stmt.childs[0]->childs;
	auto& block = *stmt.childs[1];

	auto found = _symTbl.back().find(name);
	if(found != _symTbl.back().end())
	{
		//todo message
		_errors.push_back(ErrorBuilder::Default(stmt.self.line, format("'{}': already defined", stmt.self.val)));
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
			_errors.push_back(ErrorBuilder::Default(stmt.self.line, format("'{}': already defined name is not allowed", p->self.val)));
			return false;
		}

		Param prm;
		prm.name = p->self.val;
		sym.params.push_back(prm);
	}
	_symTbl.back()[ name ] = sym;

	if(block.self == EToken::LBrace)
	{
		if(!AnalyzeCompound(block, sym.params))
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
		if(!AnalyzeStmt(*itm))
			return false;
	}

	_symTbl.pop_back();
	return true;
}
