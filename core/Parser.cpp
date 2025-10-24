#include "Parser.h"
using namespace std;



std::map<EToken, int> Parser::_opMap;
std::map<EToken, int> Parser::_precMap;

static bool InitParser()
{
	Parser::_opMap[ EToken::Assign ] = 10;
	Parser::_opMap[ EToken::PlusAssign ] = 10;
	Parser::_opMap[ EToken::MinusAssign ] = 10;
	Parser::_opMap[ EToken::MulAssign ] = 10;
	Parser::_opMap[ EToken::DivAssign ] = 10;
	Parser::_opMap[ EToken::ModAssign ] = 10;
	Parser::_opMap[ EToken::AndAssign ] = 10;
	Parser::_opMap[ EToken::OrAssign ] = 10;
	Parser::_opMap[ EToken::XorAssign ] = 10;
	Parser::_opMap[ EToken::LShiftAssign ] = 10;
	Parser::_opMap[ EToken::RShiftAssign ] = 10;

	Parser::_opMap[ EToken::Or ] = 20;
	Parser::_opMap[ EToken::And ] = 30;

	Parser::_opMap[ EToken::Pipe ] = 40;
	Parser::_opMap[ EToken::Caret ] = 50;
	Parser::_opMap[ EToken::Amp ] = 60;

	Parser::_opMap[ EToken::Equal ] = 70;
	Parser::_opMap[ EToken::NotEqual ] = 70;

	Parser::_opMap[ EToken::Greater ] = 80;
	Parser::_opMap[ EToken::Less ] = 80;
	Parser::_opMap[ EToken::GreaterEqual ] = 80;
	Parser::_opMap[ EToken::LessEqual ] = 80;

	Parser::_opMap[ EToken::LShift ] = 90;
	Parser::_opMap[ EToken::RShift ] = 90;

	Parser::_opMap[ EToken::Plus ] = 100;
	Parser::_opMap[ EToken::Minus ] = 100;

	Parser::_opMap[ EToken::Star ] = 110;
	Parser::_opMap[ EToken::Slash ] = 110;
	Parser::_opMap[ EToken::Percent ] = 110;

	/*===========================================*/

	Parser::_precMap[ EToken::Assign ] = 10;
	Parser::_precMap[ EToken::PlusAssign ] = 10;
	Parser::_precMap[ EToken::MinusAssign ] = 10;
	Parser::_precMap[ EToken::MulAssign ] = 10;
	Parser::_precMap[ EToken::DivAssign ] = 10;
	Parser::_precMap[ EToken::ModAssign ] = 10;
	Parser::_precMap[ EToken::AndAssign ] = 10;
	Parser::_precMap[ EToken::OrAssign ] = 10;
	Parser::_precMap[ EToken::XorAssign ] = 10;
	Parser::_precMap[ EToken::LShiftAssign ] = 10;
	Parser::_precMap[ EToken::RShiftAssign ] = 10;

	Parser::_precMap[ EToken::Or ] = 20;
	Parser::_precMap[ EToken::And ] = 30;

	Parser::_precMap[ EToken::Pipe ] = 40;
	Parser::_precMap[ EToken::Caret ] = 50;
	Parser::_precMap[ EToken::Amp ] = 60;

	Parser::_precMap[ EToken::Equal ] = 70;
	Parser::_precMap[ EToken::NotEqual ] = 70;

	Parser::_precMap[ EToken::Greater ] = 80;
	Parser::_precMap[ EToken::Less ] = 80;
	Parser::_precMap[ EToken::GreaterEqual ] = 80;
	Parser::_precMap[ EToken::LessEqual ] = 80;

	Parser::_precMap[ EToken::LShift ] = 90;
	Parser::_precMap[ EToken::RShift ] = 90;

	Parser::_precMap[ EToken::Plus ] = 100;
	Parser::_precMap[ EToken::Minus ] = 100;

	Parser::_precMap[ EToken::Star ] = 110;
	Parser::_precMap[ EToken::Slash ] = 110;
	Parser::_precMap[ EToken::Percent ] = 110;

	Parser::_precMap[ EToken::Not ] = 190;
	Parser::_precMap[ EToken::Tilde ] = 190;
	Parser::_precMap[ EToken::UnaryMinus ] = 190;
	Parser::_precMap[ EToken::UnaryPlus ] = 190;

 	Parser::_precMap[ EToken::LParen ] = 200;
 	Parser::_precMap[ EToken::Id ] = 200;
 	Parser::_precMap[ EToken::Num ] = 200;
 	Parser::_precMap[ EToken::Str ] = 200;
 	Parser::_precMap[ EToken::RawStr ] = 200;

	return true;
}
static bool init = InitParser();


bool Parser::IsOperator(EToken tok)
{
	return _opMap.find(tok) != _opMap.end();
}
bool Parser::IsOperator(const Token& tok)
{
	return IsOperator(tok.kind);
}

int Parser::CompPrec(EToken lhs, EToken rhs)
{
	auto lfound = _precMap.find(lhs);
	auto rfound = _precMap.find(rhs);

	if(lfound == _precMap.end())
		return 1;
	if(rfound == _precMap.end())
		return -1;

	return lfound->second - rfound->second;
}

int Parser::CompPrec(const Token& lhs, const Token& rhs)
{
	return CompPrec(lhs.kind, rhs.kind);
}



Parser::Parser(const std::vector<Token>& tokens)
	: _tokens(tokens)
	, _pos(0)
{

}

Parser::~Parser()
{

}

TreeNode* Parser::ParseExpLoop(EToken endToken /* = EToken::None */)
{
	TreeNode* ast = ParseExp(true);
	if(!ast) return nullptr;

	for( ; !IsEnd() && GetCur().kind != endToken; )
	{
		TreeNode* node = ParseExp(false);
		if(node == nullptr)
		{// 나가기
			break;
		}

		if(CompPrec(ast->self, node->self) >= 0)
		{
			node->childs.insert(node->childs.begin(), 1, ast);
			ast->parent = node;
			ast = node;
		}
		else
		{
			for(TreeNode* curNode = ast; ; curNode = ast->childs.back() )
			{
				if(CompPrec(curNode->self, node->self) >= 0)
				{
					TreeNode* parent = curNode->parent;
					node->childs.insert(node->childs.begin(), 1, curNode);
					curNode->parent = node;
					if(parent)
					{
						parent->childs.pop_back();
						parent->childs.push_back(node);
						node->parent = parent;
					}
					break;
				}

				if(curNode->childs.empty())
				{
					throw 'a';
				}
			}
		}
	}

	return ast;
}

TreeNode* Parser::ParseExp(bool first)
{
	//TODO
	//ParsePostfixExp();

	TreeNode* node;


	auto& cur = GetCur();
	auto& prev = GetPrev();
	if(cur.IsPrefixUnary() && (prev.IsPrefixUnary() || IsOperator(prev) || first))
	{
		if(node = ParsePrefixExp()) return node;
	}

	if(node = ParseOpExp()) return node;

	if(node = ParsePrimaryExp()) return node;

	return nullptr;
}

TreeNode* Parser::ParsePrimaryExp()
{
	const Token& cur = GetCur();
	if(cur.kind == EToken::LParen)
	{
		TreeNode* node = new TreeNode();
		node->self = cur;
		MoveNext();
		TreeNode* child = ParseExpLoop(EToken::RParen);
		if(!child)
		{
			throw 'n';
		}
		if(GetCur().kind != EToken::RParen)
		{
			throw 'n';
		}
		MoveNext();

		node->childs.push_back(child);
		child->parent = node;
		return node;
	}

	if(cur.kind == EToken::Id)
	{
		TreeNode* node = new TreeNode();
		node->self = cur;
		MoveNext();
		return node;
	}

	if(cur.IsLiteral())
	{
		TreeNode* node = new TreeNode();
		node->self = cur;
		MoveNext();
		return node;
	}

	return nullptr;
}

TreeNode* Parser::ParsePrefixExp()
{
	auto& cur = GetCur();
	if(!cur.IsPrefixUnary())
	{
		return nullptr;
	}

	MoveNext();

	TreeNode* rhs = ParsePrefixExp();
	if(!rhs) rhs = ParsePrimaryExp();
	if(rhs != nullptr)
	{
		TreeNode* node = new TreeNode;
		node->self = cur;
		if(cur.kind == EToken::Plus) node->self.kind = EToken::UnaryPlus;
		if(cur.kind == EToken::Minus) node->self.kind = EToken::UnaryMinus;

		node->childs.push_back(rhs);
		rhs->parent = node;
		return node;
	}
	else
	{
		throw 'n';
	}

	return nullptr;
}

TreeNode* Parser::ParseOpExp()
{
	auto& cur = GetCur();
	auto found = _opMap.find(cur.kind);
	if(found == _opMap.end())
	{
		return nullptr;
	}

	MoveNext();

	TreeNode* rhs = ParseExp(false);
	if(rhs != nullptr)
	{
		TreeNode* node = new TreeNode();
		node->self = cur;
		node->childs.push_back(rhs);
		rhs->parent = node;
		return node;
	}
	else
	{
		throw 'n';
	}

	return nullptr;
}


TreeNode* Parser::ParseCompoundStmt()
{
	if(GetCur().kind != EToken::LBrace)
	{
		return nullptr;
	}

	TreeNode* compound = new TreeNode;
	compound->self = GetCur();
	MoveNext();

	for( ; GetCur().kind != EToken::RBrace; )
	{
		if(IsEnd())
		{
			throw 'n';
		}

		TreeNode* stmt = ParseStmt();
		if(!stmt)
		{
			throw 'n';
		}

		compound->childs.push_back(stmt);
		stmt->parent = compound;
	}

	MoveNext();

	return compound;
}

TreeNode* Parser::ParseStmt()
{
	TreeNode* ast = ParseExpLoop();
	if(ast) return ast;

	if(ast = ParseCompoundStmt()) return ast;

	if(ast = ParseIf()) return ast;
	if(ast = ParseFor()) return ast;
	if(ast = ParseFn()) return ast;

	return nullptr;
}

TreeNode* Parser::ParseIf()
{
	if(GetCur().kind != EToken::If)
	{
		return nullptr;
	}

	Token _if = GetCur();
	MoveNext();

	if(GetCur().kind != EToken::LParen)
	{
		throw 'n';
	}
	MoveNext();

	TreeNode* exp = ParseExpLoop(EToken::RParen);
	if(!exp)
	{
		throw 'n';
	}
	if(GetCur().kind != EToken::RParen)
	{
		throw 'n';
	}
	MoveNext();

	TreeNode* ifNode = new TreeNode;
	ifNode->self = _if;
	ifNode->childs.push_back(exp);
	exp->parent = ifNode;

	TreeNode* _true = ParseStmt();
	if(!_true)
	{
		throw 'n';
	}

	ifNode->childs.push_back(_true);
	_true->parent = ifNode;

	if(GetCur().kind == EToken::Else)
	{
		MoveNext();
		TreeNode* _false = ParseStmt();
		if(!_true)
		{
			throw 'n';
		}

		ifNode->childs.push_back(_false);
		_false->parent = ifNode;
	}

	return ifNode;
}

TreeNode* Parser::ParseFor()
{
	if(GetCur().kind != EToken::For)
	{
		return nullptr;
	}

	Token _for = GetCur();
	MoveNext();

	if(GetCur().kind != EToken::LParen)
	{
		throw 'n';
	}
	MoveNext();

	//TODO 메모리 릭
	TreeNode* init = ParseExpLoop(EToken::Semicolon);
	if(!init)
	{
		init = new TreeNode();
		init->self = { EToken::Num, _for.line, "1" };
	}
	if(GetCur().kind != EToken::Semicolon)
	{
		throw 'n';
	}
	MoveNext();

	//TODO 메모리 릭
	TreeNode* cond = ParseExpLoop(EToken::Semicolon);
	if(!cond)
	{
		cond = new TreeNode();
		cond->self = { EToken::Num, _for.line, "1" };
	}
	if(GetCur().kind != EToken::Semicolon)
	{
		throw 'n';
	}
	MoveNext();

	//TODO 메모리 릭
	TreeNode* update = ParseExpLoop(EToken::RParen);
	if(!update)
	{
		update = new TreeNode();
		update->self = { EToken::Num, _for.line, "1" };
	}
	if(GetCur().kind != EToken::RParen)
	{
		throw 'n';
	}
	MoveNext();

	TreeNode* forNode = new TreeNode;
	forNode->self = _for;
	forNode->childs.push_back(init);
	forNode->childs.push_back(cond);
	forNode->childs.push_back(update);
	init->parent = forNode;
	cond->parent = forNode;
	update->parent = forNode;

	TreeNode* loop = ParseStmt();
	if(!loop)
	{
		throw 'n';
	}

	forNode->childs.push_back(loop);
	loop->parent = forNode;
	return forNode;
}

TreeNode* Parser::ParseFn()
{
	if(GetCur().kind != EToken::Fn)
	{
		return nullptr;
	}

	Token fn = GetCur();
	MoveNext();

	if(GetCur().kind != EToken::Id)
	{
		throw 'n';
	}
	MoveNext();

	if(GetCur().kind != EToken::LParen)
	{
		throw 'n';
	}
	MoveNext();

	TreeNode* params = new TreeNode;
	params->self.line = fn.line;
	//TODO 메모리 릭
	for( ; ; )
	{
		if(GetCur().kind == EToken::RParen)
		{//push
			MoveNext();
			break;
		}
		else if(GetCur().kind == EToken::Id)
		{
			TreeNode* param = new TreeNode;
			param->self = GetCur();
			params->childs.push_back(param);
			param->parent = params;
			MoveNext();
			if(GetCur().kind == EToken::Comma)
			{
				MoveNext();
			}
			else if(GetCur().kind != EToken::RParen)
			{
				throw 'n';
			}
		}
		else
		{
			throw 'n';
		}
	}

	TreeNode* body = ParseStmt();
	if(!body)
	{
		throw 'n';
	}

	TreeNode* fnNode = new TreeNode;
	fnNode->childs.push_back(params);
	fnNode->childs.push_back(body);
	body->parent = fnNode;
	return fnNode;
}


TreeNode* Parser::Parse()
{
	TreeNode* root = new TreeNode;

	for( ; !IsEnd(); )
	{
		TreeNode* ast = ParseStmt();
		if(!ast)
		{
			throw 'n';
		}
		root->childs.push_back(ast);
		ast->parent = root;
	}
	return root;
}


bool Parser::MoveNext()
{
	if(_pos >= _tokens.size()) return true;

	_pos++;
	return true;
}
bool Parser::MovePrev()
{
	if(_pos == 0) return false;

	_pos--;
	return true;
}
bool Parser::IsEnd() const
{
	return _pos >= _tokens.size();
}
const Token& Parser::GetPrev() const
{
	static Token nullToken;
	return _pos > 0 && _pos-1 < _tokens.size() ? _tokens[_pos-1] : nullToken;
}

const Token& Parser::GetCur() const
{
	static Token nullToken;
	return _pos < _tokens.size() ? _tokens[_pos] : nullToken;
}
