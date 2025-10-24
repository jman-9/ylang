#include "Parser.h"
using namespace std;


static map<EToken, int> s_opMap;
static map<EToken, int> s_precMap;
static vector<EToken> s_allowedFor;
static vector<EToken> s_allowedFn;

static bool InitParser()
{
	s_opMap[ EToken::Assign ] = 10;
	s_opMap[ EToken::PlusAssign ] = 10;
	s_opMap[ EToken::MinusAssign ] = 10;
	s_opMap[ EToken::MulAssign ] = 10;
	s_opMap[ EToken::DivAssign ] = 10;
	s_opMap[ EToken::ModAssign ] = 10;
	s_opMap[ EToken::AndAssign ] = 10;
	s_opMap[ EToken::OrAssign ] = 10;
	s_opMap[ EToken::XorAssign ] = 10;
	s_opMap[ EToken::LShiftAssign ] = 10;
	s_opMap[ EToken::RShiftAssign ] = 10;

	s_opMap[ EToken::Or ] = 20;
	s_opMap[ EToken::And ] = 30;

	s_opMap[ EToken::Pipe ] = 40;
	s_opMap[ EToken::Caret ] = 50;
	s_opMap[ EToken::Amp ] = 60;

	s_opMap[ EToken::Equal ] = 70;
	s_opMap[ EToken::NotEqual ] = 70;

	s_opMap[ EToken::Greater ] = 80;
	s_opMap[ EToken::Less ] = 80;
	s_opMap[ EToken::GreaterEqual ] = 80;
	s_opMap[ EToken::LessEqual ] = 80;

	s_opMap[ EToken::LShift ] = 90;
	s_opMap[ EToken::RShift ] = 90;

	s_opMap[ EToken::Plus ] = 100;
	s_opMap[ EToken::Minus ] = 100;

	s_opMap[ EToken::Star ] = 110;
	s_opMap[ EToken::Slash ] = 110;
	s_opMap[ EToken::Percent ] = 110;

	/*===========================================*/

	s_precMap[ EToken::Assign ] = 10;
	s_precMap[ EToken::PlusAssign ] = 10;
	s_precMap[ EToken::MinusAssign ] = 10;
	s_precMap[ EToken::MulAssign ] = 10;
	s_precMap[ EToken::DivAssign ] = 10;
	s_precMap[ EToken::ModAssign ] = 10;
	s_precMap[ EToken::AndAssign ] = 10;
	s_precMap[ EToken::OrAssign ] = 10;
	s_precMap[ EToken::XorAssign ] = 10;
	s_precMap[ EToken::LShiftAssign ] = 10;
	s_precMap[ EToken::RShiftAssign ] = 10;

	s_precMap[ EToken::Or ] = 20;
	s_precMap[ EToken::And ] = 30;

	s_precMap[ EToken::Pipe ] = 40;
	s_precMap[ EToken::Caret ] = 50;
	s_precMap[ EToken::Amp ] = 60;

	s_precMap[ EToken::Equal ] = 70;
	s_precMap[ EToken::NotEqual ] = 70;

	s_precMap[ EToken::Greater ] = 80;
	s_precMap[ EToken::Less ] = 80;
	s_precMap[ EToken::GreaterEqual ] = 80;
	s_precMap[ EToken::LessEqual ] = 80;

	s_precMap[ EToken::LShift ] = 90;
	s_precMap[ EToken::RShift ] = 90;

	s_precMap[ EToken::Plus ] = 100;
	s_precMap[ EToken::Minus ] = 100;

	s_precMap[ EToken::Star ] = 110;
	s_precMap[ EToken::Slash ] = 110;
	s_precMap[ EToken::Percent ] = 110;

	s_precMap[ EToken::Not ] = 180;
	s_precMap[ EToken::Tilde ] = 180;
	s_precMap[ EToken::UnaryMinus ] = 180;
	s_precMap[ EToken::UnaryPlus ] = 180;

	s_precMap[ EToken::Invoke ] = 190;
	s_precMap[ EToken::Dot ] = 190;
	s_precMap[ EToken::LBracket ] = 190;

 	s_precMap[ EToken::LParen ] = 200;
 	s_precMap[ EToken::Id ] = 200;
 	s_precMap[ EToken::Num ] = 200;
 	s_precMap[ EToken::Str ] = 200;
 	s_precMap[ EToken::RawStr ] = 200;

	/*===========================================*/

	s_allowedFor.push_back(EToken::Break);
	s_allowedFor.push_back(EToken::Continue);
	s_allowedFn.push_back(EToken::Return);

	return true;
}
static bool init = InitParser();


static bool IsOperator(EToken tok)
{
	return s_opMap.find(tok) != s_opMap.end();
}
static bool IsOperator(const Token& tok)
{
	return IsOperator(tok.kind);
}
static bool IsPrimary(EToken tok)
{
	return tok == EToken::Id || Token::IsLiteral(tok);
}
static bool IsPrimary(const Token& tok)
{
	return IsPrimary(tok.kind);
}

static int CompPrec(EToken lhs, EToken rhs)
{
	auto lfound = s_precMap.find(lhs);
	auto rfound = s_precMap.find(rhs);

	if(lfound == s_precMap.end())
		return 1;
	if(rfound == s_precMap.end())
		return -1;

	return lfound->second - rfound->second;
}

static int CompPrec(const Token& lhs, const Token& rhs)
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

TreeNode* Parser::ParseExpLoop(EToken endToken /* = EToken::None */, EToken endToken2 /* = EToken::None */)
{
	TreeNode* ast = ParseExp(true);
	if(!ast) return nullptr;

	for( ; !IsEnd() && GetCur().kind != endToken && GetCur().kind != endToken2; )
	{
		TreeNode* node = ParseExp(false);
		if(node == nullptr)
		{
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
			for(TreeNode* curNode = ast; ; curNode = curNode->childs.back() )
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
	TreeNode* node;

	if(GetCur().kind == EToken::LParen)
	{
		if(!IsPrimary(GetPrev()) && GetPrev().kind != EToken::RParen)
			if(node = ParsePrimaryExp()) return node;
	}
	else
	{
		if(node = ParsePrimaryExp()) return node;
	}

	if(node = ParsePostfixExp()) return node;

	auto& cur = GetCur();
	auto& prev = GetPrev();
	if(cur.IsPrefixUnary() && (prev.IsPrefixUnary() || IsOperator(prev) || first))
	{
		if(node = ParsePrefixExp()) return node;
	}

	if(node = ParseOpExp()) return node;

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
	else if(IsPrimary(cur))
	{
		TreeNode* node = new TreeNode();
		node->self = cur;
		MoveNext();
		return node;
	}

	return nullptr;
}

TreeNode* Parser::ParsePostfixExp()
{
	const Token& cur = GetCur();
	if(cur.kind == EToken::LParen)
	{
		TreeNode* args = new TreeNode;
		args->self.kind = EToken::Invoke;
		//todo
		args->self.val = "invoke";
		args->self.line = cur.line;
		MoveNext();

		for( ; ; )
		{
			TreeNode* arg = ParseExpLoop(EToken::Comma, EToken::RParen);
			if(arg)
			{
				args->childs.push_back(arg);
				arg->parent = args;
			}

			MoveNext();
			if(GetPrev().kind == EToken::RParen)
			{
				break;
			}
			else if(GetPrev().kind != EToken::Comma)
			{
				//TODO 메모리 릭
				throw 'n';
			}
		}
		return args;
	}
	else if(cur.kind == EToken::LBracket)
	{
		TreeNode* idx = new TreeNode;
		idx->self = cur;
		MoveNext();

		TreeNode* val = ParseExpLoop(EToken::RBracket);
		MoveNext();
		if(GetPrev().kind != EToken::RBracket)
		{//TODO 메모리 릭
			throw 'n';
		}

		idx->childs.push_back(val);
		val->parent = idx;
		return idx;
	}
	else if(cur.kind == EToken::Dot)
	{
		Token acc = cur;

		MoveNext();
		if(GetCur().kind != EToken::Id)
		{
			throw 'n';
		}

		TreeNode* id = ParsePrimaryExp();
		TreeNode* node = new TreeNode;
		node->self = acc;
		node->childs.push_back(id);
		id->parent = node;
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
	auto found = s_opMap.find(cur.kind);
	if(found == s_opMap.end())
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


TreeNode* Parser::ParseCompoundStmt(const std::vector<EToken>& allowed /* = std::vector<EToken>() */)
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

		TreeNode* stmt = ParseStmt(allowed);
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

TreeNode* Parser::ParseStmt(const std::vector<EToken>& allowed /* = std::vector<EToken>() */)
{
	TreeNode* ast = ParseExpLoop(EToken::Semicolon);
	if(ast)
	{
		if(GetCur().kind != EToken::Semicolon)
		{
			throw 'n';
		}
		MoveNext();
		if(ast) return ast;
	}

	if(ast = ParseCompoundStmt(allowed)) return ast;

	if(ast = ParseIf()) return ast;
	if(ast = ParseFor()) return ast;
	if(ast = ParseFn()) return ast;

	auto& cur = GetCur();
	bool needParsing = false;
	for(auto a : allowed)
	{
		if(a == cur.kind)
		{
			needParsing = true;
			break;
		}
	}
	if(!needParsing) return nullptr;

	if(cur.kind == EToken::Continue || cur.kind == EToken::Break)
	{
		if(GetNext().kind != EToken::Semicolon)
		{
			throw 'n';
		}
		ast = new TreeNode;
		ast->self = cur;
		MoveNext();
		MoveNext();
		return ast;
	}
	else if(cur.kind == EToken::Return)
	{
		ast = new TreeNode;
		ast->self = cur;
		MoveNext();
		TreeNode* ret = ParseExpLoop();
		if(ret)
		{
			ast->childs.push_back(ret);
			ret->parent = ast;
		}

		if(GetCur().kind != EToken::Semicolon)
		{
			throw 'n';
		}
		MoveNext();

		return ast;
	}

	throw 'n';
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

	TreeNode* loop = ParseStmt(s_allowedFor);
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

	TreeNode* body = ParseStmt(s_allowedFn);
	if(!body)
	{
		throw 'n';
	}

	TreeNode* fnNode = new TreeNode;
	fnNode->self = fn;
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

static Token nullToken;
const Token& Parser::GetPrev() const
{
	return _pos > 0 && _pos-1 < _tokens.size() ? _tokens[_pos-1] : nullToken;
}
const Token& Parser::GetCur() const
{
	return _pos < _tokens.size() ? _tokens[_pos] : nullToken;
}
const Token& Parser::GetNext() const
{
	return _pos+1 < _tokens.size() ? _tokens[_pos+1] : nullToken;
}
