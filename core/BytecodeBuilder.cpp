#include "BytecodeBuilder.h"
#include <format>
#include <iostream>
using namespace std;


BytecodeBuilder::BytecodeBuilder(const TreeNode& code)
	: _code(code)
	, _reg(0)
{
	_symTbl.resize(1);
}

BytecodeBuilder::~BytecodeBuilder()
{

}

bool BytecodeBuilder::Build()
{
	for(const auto& stmt : _code.childs)
	{
		BuildStmt(*stmt);
	}


	for(int i=0; i<_bytecode.size(); i++)
	{
		cout << format("{:4} {}\n", i+1, _bytecode[i].codeStr);
	}

	return true;
}

bool BytecodeBuilder::BuildStmt(const TreeNode& stmt)
{
	switch(stmt.self.kind)
	{
	case EToken::For : return BuildFor(stmt);
	case EToken::If : return BuildIf(stmt);
	case EToken::Fn : return BuildFn(stmt);
	case EToken::LBrace : return BuildCompound(stmt);
	case EToken::Return :
	case EToken::Continue :
	case EToken::Break : return true;
	default: ;
	}
	return BuildExp(stmt, true);
}

bool BytecodeBuilder::BuildExp(const TreeNode& stmt, bool root)
{
	uint32_t regStack = _reg;

	if(stmt.self == EToken::Id || stmt.self.IsLiteral())
	{
		_bytecode.push_back( { .codeStr = format("{} = {}", format("t{}", _reg), stmt.self.val) } );
		return true;
	}

	if(stmt.self == EToken::Invoke)
	{
		for(size_t i = 1; i<stmt.childs	.size(); i++)
		{
			if(!BuildExp(*stmt.childs[i], false))
				throw 'n';
			_reg++;
		}

		_reg = regStack;
		_bytecode.push_back( { .codeStr = format("t{} = invoke {}", _reg, _symTbl.back()[stmt.childs[0]->self.val].pos) } );
		return true;
	}


	TreeNode* lhs = !stmt.childs.empty() ? stmt.childs.front() : nullptr;
	TreeNode* rhs = stmt.childs.size() > 1 ? stmt.childs.back() : nullptr;


	if(!lhs)
	{
		throw 'n';
	}

	string lhsStr;
	string rhsStr;
	string codeStr;

	if(lhs->self != EToken::Id && !lhs->self.IsLiteral())
	{
		if(!BuildExp(*lhs, false))
		{
			throw 'n';
		}
		lhsStr = format("t{}", _reg);
		_reg++;
	}
	else
	{
		lhsStr = lhs->self.val;
	}

	if(rhs)
	{
		if(rhs->self != EToken::Id && !rhs->self.IsLiteral())
		{
			if(!BuildExp(*rhs, false))
			{
				throw 'n';
			}
			rhsStr = format("t{}", _reg);
			_reg++;
		}
		else
		{
			rhsStr = rhs->self.val;
		}

		codeStr = format("{} {} {}", lhsStr, stmt.self.val, rhsStr);
	}
	else
	{
		if(stmt.self.IsPrefixUnary())
		{
			codeStr = stmt.self.val + lhsStr;
		}
		else if(stmt.self == EToken::LParen)
		{
			codeStr = lhsStr;
		}
		else
		{
			throw 'n';
		}
	}

	_reg = regStack;
	if(!root)
	{
		_bytecode.push_back( { .codeStr = format("{} = {}", format("t{}", _reg), codeStr) } );
	}
	else
	{
		_bytecode.push_back( { .codeStr = codeStr } );
	}

	return true;
}

bool BytecodeBuilder::BuildFor(const TreeNode& stmt)
{
	if(stmt.self != EToken::For)
		throw 'n';

	auto& init = *stmt.childs[0];
	auto& cond = *stmt.childs[1];
	auto& update = *stmt.childs[2];
	auto& block = *stmt.childs[3];

	string condStr;

	BuildExp(init, true);

	size_t loopStart = _bytecode.size() + 1;
	BuildExp(cond, false);
	condStr = format("jne {}, ", format("t{}", _reg));
	_bytecode.push_back( { .codeStr = condStr } );
	size_t condLine = _bytecode.size() - 1;


	BuildCompound(block);

	size_t loopEnd = _bytecode.size();
	BuildExp(update, true);

	_bytecode.push_back( { .codeStr = format("jmp {}", loopStart) } );

	size_t updateEnd = _bytecode.size() + 1;

	_bytecode[condLine].codeStr += to_string(updateEnd);

	return true;
}

bool BytecodeBuilder::BuildIf(const TreeNode& stmt)
{
	if(stmt.self != EToken::If)
		throw 'n';

	auto& test = *stmt.childs[0];
	auto& _true = *stmt.childs[1];

	string condStr;

	BuildExp(test, false);
	condStr = format("jne {}, ", format("t{}", _reg));
	_bytecode.push_back( { .codeStr = condStr } );
	size_t condLine = _bytecode.size() - 1;

	BuildStmt(_true);

	_bytecode[condLine].codeStr += to_string(_bytecode.size() + 1);

	if(stmt.childs.size() > 2)
	{
		auto& _false = *stmt.childs[2];
		if(!BuildStmt(_false))
		{
			throw 'n';
		}
	}
	return true;
}

bool BytecodeBuilder::BuildFn(const TreeNode& stmt)
{
	uint32_t regStack = _reg;
	_reg = 0;

	if(stmt.self != EToken::Fn)
		throw 'n';

	size_t skipLine = _bytecode.size();
	_bytecode.push_back( { .codeStr = "jmp " } );

	auto& name = stmt.self.val;
	auto& params = stmt.childs[0]->childs;
	auto& block = *stmt.childs[1];

	Symbol sym;
	sym.name = name;
	sym.pos = _bytecode.size() + 1;
	sym.kind = ESymbol::Fn;
	for(auto& p : params)
	{
		Param prm;
		prm.name = p->self.val;
		sym.params.push_back(prm);

		_bytecode.push_back( { .codeStr = format("{} = t{}", prm.name, _reg++) } );
	}

	_reg = 1;
	if(block.self == EToken::LBrace)
	{
		if(!BuildCompound(block))
		{
			throw 'n';
		}
	}
	else if(!BuildStmt(block))
	{
		throw 'n';
	}

	_bytecode.push_back( { .codeStr = "ret" } );

	_bytecode[skipLine].codeStr += to_string(_bytecode.size()+1);

	_reg = regStack;

	//TODO
	_symTbl.back()[ name ] = sym;
	return true;
}

bool BytecodeBuilder::BuildCompound(const TreeNode& stmt)
{
	if(stmt.self != EToken::LBrace)
		throw 'n';

	_symTbl.resize(_symTbl.size() + 1);
	_symTbl.back() = _symTbl[_symTbl.size() - 2];

	for(auto& itm : stmt.childs)
	{
		BuildStmt(*itm);
	}

	_symTbl.pop_back();
	return true;
}
