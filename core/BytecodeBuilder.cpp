#include "BytecodeBuilder.h"
#include <format>
#include <iostream>
using namespace std;


BytecodeBuilder::BytecodeBuilder(const TreeNode& code)
	: _code(code)
	, _reg(0)
{

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
	default: ;
	}
	return BuildExp(stmt, true);
}

bool BytecodeBuilder::BuildExp(const TreeNode& stmt, bool root)
{
	uint32_t regStack = _reg;

	if(stmt.self == EToken::Invoke)
	{
		//BuildInvoke(stmt);
		// 0번이 함수 이름 리졸브

		int a = 1;
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

	_bytecode.push_back( { .codeStr = format("goto {}", loopStart) } );

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

	return true;
}

bool BytecodeBuilder::BuildCompound(const TreeNode& stmt)
{
	if(stmt.self != EToken::LBrace)
		throw 'n';

	for(auto& itm : stmt.childs)
	{
		BuildStmt(*itm);
	}

	return true;
}
