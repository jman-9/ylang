#include "Parser.h"
#include <queue>
using namespace std;


#define NewNode TreeNode::New

static map<EToken, int> s_opMap;
static map<EToken, int> s_precMap;
static set<EToken> s_allowedFor;
static set<EToken> s_allowedFn;

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
	s_precMap[ EToken::PreInc ] = 180;
	s_precMap[ EToken::PreDec ] = 180;

	s_precMap[ EToken::Invoke ] = 190;
	s_precMap[ EToken::Dot ] = 190;
	s_precMap[ EToken::Index ] = 190;
	s_precMap[ EToken::PostInc ] = 190;
	s_precMap[ EToken::PostDec ] = 190;

 	s_precMap[ EToken::LParen ] = 200;
 	s_precMap[ EToken::Id ] = 200;
 	s_precMap[ EToken::Int ] = 200;
	s_precMap[ EToken::Float ] = 200;
 	s_precMap[ EToken::Str ] = 200;
 	s_precMap[ EToken::RawStr ] = 200;
	s_precMap[ EToken::List ] = 200;

	/*===========================================*/

	s_allowedFor.insert(EToken::Break);
	s_allowedFor.insert(EToken::Continue);
	s_allowedFn.insert(EToken::Return);

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
static bool IsPrimaryPrefix(EToken tok)
{
	return tok == EToken::LParen || tok == EToken::LBracket || tok == EToken::LBrace || tok == EToken::Id || Token::IsLiteral(tok);
}
static bool IsPrimaryPrefix(const Token& tok)
{
	return IsPrimaryPrefix(tok.kind);
}
static bool IsPrimaryPostfix(EToken tok)
{
	return tok == EToken::RParen || tok == EToken::RBracket || tok == EToken::RBrace || tok == EToken::Id || Token::IsLiteral(tok);
}
static bool IsPrimaryPostfix(const Token& tok)
{
	return IsPrimaryPostfix(tok.kind);
}
static bool IsLValue(EToken tok)
{
	return tok == EToken::Id || tok == EToken::Dot || tok == EToken::Index || tok == EToken::LValueIndex || tok == EToken::LValueField;
}
static bool IsLValue(const Token& tok)
{
	return IsLValue(tok.kind);
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

static int CompPrec(const shared_ptr<TreeNode>& lhs, const shared_ptr<TreeNode>& rhs)
{
	int lpri = lhs->priority;
	if(!lpri)
	{
		auto lfound = s_precMap.find(lhs->self.kind);
		if(lfound == s_precMap.end())
			return 1;
		lpri = lfound->second;
	}

	int rpri = rhs->priority;
	if(!rpri)
	{
		auto rfound = s_precMap.find(rhs->self.kind);
		if(rfound == s_precMap.end())
			return -1;
		rpri = rfound->second;
	}

	return lpri - rpri;
}



Parser::Parser(const std::vector<Token>& tokens)
	: _tokens(tokens)
	, _pos(0)
{

}

Parser::~Parser()
{

}

TreeNodeSptr Parser::ParseExpLoop(EToken endToken /* = EToken::None */, EToken endToken2 /* = EToken::None */)
{
	TreeNodeSptr root = ParseExp(true);
	if(!root) return nullptr;

	for( ; !IsEnd() && GetCur().kind != endToken && GetCur().kind != endToken2; )
	{
		TreeNodeSptr node = ParseExp(false);
		if(node == nullptr)
		{
			break;
		}

		for(TreeNodeSptr curNode = root; ; curNode = curNode->childs.back())
		{
			int prec = CompPrec(curNode, node);
			if(prec > 0 || (prec == 0 && (!curNode->self.IsAssign() && !node->self.IsAssign())))
			{
				TreeNode* parent = curNode->parent;
				node->PushFrontChild(curNode);
				if(parent) parent->ReplaceBackChild(node);
				if(curNode == root) root = node;
				break;
			}

			if(curNode->childs.empty())
			{
				throw 'a';
			}
		}
	}

	queue<TreeNodeSptr> q;
	q.push(root);
	for( ; !q.empty(); )
	{
		TreeNodeSptr cur = q.front();
		q.pop();

		if(cur->self.IsAssign() || cur->self.IsIncDecOp())
		{
			auto& LValue = cur->childs.front();
			if(!IsLValue(LValue->self))
			{
				_errors.push_back(ErrorBuilder::LValueError(LValue->self.line, LValue->self.val));
				return nullptr;
			}

			if(LValue->self == EToken::Index)
				LValue->self.kind = EToken::LValueIndex;
			else if(LValue->self == EToken::Dot)
				LValue->self.kind = EToken::LValueField;
		}

		for(auto& n : cur->childs)
			q.push(n);
	}

	return root;
}

TreeNodeSptr Parser::ParseExp(bool first)
{
	TreeNodeSptr node;

	for( ; ; )
	{
		if(GetCur() == EToken::LParen)
		{
			if(!IsPrimaryPostfix(GetPrev()))
				if(node = ParsePrimaryExp()) break;
		}
		else if(GetCur() == EToken::LBracket)
		{
			if(!IsPrimaryPostfix(GetPrev()))
				if(node = ParsePrimaryExp()) break;
		}
		else
		{
			if(node = ParsePrimaryExp()) break;
		}

		if(node = ParsePostfixExp()) break;

		//TODO modify logic
		auto& cur = GetCur();
		auto& prev = GetPrev();
		if(cur.IsPrefixUnary() && ((prev.IsPrefixUnary() && !prev.IsIncDecOp()) || IsOperator(prev) || first))
		{//TODO clarify prefix parse condition
			if(node = ParsePrefixExp()) break;
		}

		node = ParseOpExp();
		break;
	}

	if(node)
	{
		queue<TreeNodeSptr> q;
		q.push(node);
		for( ; !q.empty(); )
		{
			TreeNodeSptr cur = q.front();
			q.pop();

			if(cur->self == EToken::LParen)
			{
				auto overridedChild = cur->childs.front();

				//TODO only 1?
				overridedChild->priority = s_precMap[EToken::LParen];

				if(cur->parent)
				{
					auto parent = cur->parent;
					parent->ReplaceChild(cur, overridedChild);
				}
				else
				{
					overridedChild->parent = nullptr;
				}

				if(cur == node)
				{
					node = overridedChild;
				}

				cur = overridedChild;
			}

			for(auto& n : cur->childs)
				q.push(n);
		}
	}

	return node;
}

TreeNodeSptr Parser::ParsePrimaryExp()
{
	const Token& cur = GetCur();
	if(!IsPrimaryPrefix(cur))
		return nullptr;

	if(cur.kind == EToken::LParen)
	{
		int stack = _pos;
		TreeNodeSptr node = NewNode();
		node->self = cur;
		MoveNext();
		TreeNodeSptr child = ParseExpLoop(EToken::RParen);
		if(!child)
		{
			_pos = stack;
			return nullptr;
		}
		if(GetCur().kind != EToken::RParen)
		{
			_pos = stack;
			return nullptr;
		}
		MoveNext();

		node->PushBackChild(child);
		return node;
	}
	else if(cur.kind == EToken::LBracket)
	{//list
		TreeNodeSptr node = NewNode();
		node->self = cur;
		node->self.kind = EToken::List;
		MoveNext();

		for( ; ; )
		{
			if(GetCur().kind == EToken::RBracket)
			{//push
				MoveNext();
				break;
			}

			TreeNodeSptr child = ParseExpLoop(EToken::Comma, EToken::RBracket);
			if(!child)
			{
				_errors.push_back(ErrorBuilder::SyntaxError(cur.line, ','));
				return nullptr;
			}
			node->PushBackChild(child);
			if(GetCur().kind == EToken::RBracket)
			{//push
				MoveNext();
				break;
			}

			MoveNext();
		}
		return node;
	}
	else if(cur.kind == EToken::LBrace)
	{
		TreeNodeSptr node = NewNode();
		node->self = cur;
		node->self.kind = EToken::Dict;
		MoveNext();

		for( ; ; )
		{
			if(GetCur().kind == EToken::RBrace)
			{//push
				MoveNext();
				break;
			}

			TreeNodeSptr child = ParseExpLoop(EToken::Colon);
			if(!child)
			{
				_errors.push_back(ErrorBuilder::SyntaxError(cur.line, ':'));
				return nullptr;
			}
			node->PushBackChild(child);
			MoveNext();

			child = ParseExpLoop(EToken::Comma, EToken::RBrace);
			if(!child)
			{
				_errors.push_back(ErrorBuilder::SyntaxError(cur.line, ','));
				return nullptr;
			}
			node->childs.back()->PushBackChild(child);
			if(GetCur().kind == EToken::RBrace)
			{//push
				MoveNext();
				break;
			}

			MoveNext();
		}
		return node;
	}
	else
	{
		TreeNodeSptr node = NewNode();
		node->self = cur;
		MoveNext();
		return node;
	}

	return nullptr;
}

TreeNodeSptr Parser::ParsePostfixExp()
{
	const Token prev = GetPrev();
	if(!IsPrimaryPostfix(prev))
		return nullptr;

	const Token cur = GetCur();
	if(cur.kind == EToken::LParen)
	{
		TreeNodeSptr args = NewNode();
		args->self.kind = EToken::Invoke;
		//todo
		args->self.val = "invoke";
		args->self.line = cur.line;
		MoveNext();

		for( ; ; )
		{
			TreeNodeSptr arg = ParseExpLoop(EToken::Comma, EToken::RParen);
			if(arg) args->PushBackChild(arg);

			MoveNext();
			if(GetPrev().kind == EToken::RParen)
			{
				break;
			}
			else if(GetPrev().kind != EToken::Comma)
			{
				_errors.push_back(ErrorBuilder::Missing(cur.line, ','));
				return nullptr;
			}
		}
		return args;
	}
	else if(cur.kind == EToken::LBracket)
	{
		TreeNodeSptr idx = NewNode();
		idx->self = cur;
		idx->self.kind = EToken::Index;
		MoveNext();

		TreeNodeSptr val = ParseExpLoop(EToken::RBracket);
		MoveNext();
		if(GetPrev().kind != EToken::RBracket)
		{
			_errors.push_back(ErrorBuilder::Missing(cur.line, ']'));
			return nullptr;
		}

		idx->PushBackChild(val);
		return idx;
	}
	else if(cur.kind == EToken::Dot)
	{
		Token acc = cur;

		MoveNext();
		if(GetCur().kind != EToken::Id)
		{	//TODO classify
			_errors.push_back(ErrorBuilder::SyntaxError(cur.line, '.'));
			return nullptr;
		}

		TreeNodeSptr id = ParsePrimaryExp();
		if(id->self == EToken::Id)
		{//to change attr str
			id->self.kind = EToken::Str;
		}

		TreeNodeSptr node = NewNode();
		node->self = acc;
		node->PushBackChild(id);
		return node;
	}
	else if(cur == EToken::PlusPlus || cur == EToken::MinusMinus)
	{
		TreeNodeSptr node = NewNode();
		node->self = cur;
		node->self.kind = cur == EToken::PlusPlus ? EToken::PostInc : EToken::PostDec;
		MoveNext();
		return node;
	}

	return nullptr;
}

TreeNodeSptr Parser::ParsePrefixExp()
{
	//TODO modify logic

	auto& cur = GetCur();
	if(!cur.IsPrefixUnary())
	{
		return nullptr;
	}

	int stack = _pos;
	MoveNext();

	TreeNodeSptr rhs = ParsePrefixExp();
	if(!rhs) rhs = ParsePrimaryExp();
	if(!rhs) { _pos = stack; return nullptr; }

	TreeNodeSptr node = NewNode();
	node->self = cur;
	if(cur.kind == EToken::Plus) node->self.kind = EToken::UnaryPlus;
	if(cur.kind == EToken::Minus) node->self.kind = EToken::UnaryMinus;
	if(cur.kind == EToken::PlusPlus) node->self.kind = EToken::PreInc;
	if(cur.kind == EToken::MinusMinus) node->self.kind = EToken::PreDec;

	node->PushBackChild(rhs);
	return node;
}

TreeNodeSptr Parser::ParseOpExp()
{
	auto& cur = GetCur();
	auto found = s_opMap.find(cur.kind);
	if(found == s_opMap.end())
	{
		return nullptr;
	}

	MoveNext();

	TreeNodeSptr rhs = ParseExp(false);
	if(!rhs) return nullptr;

	TreeNodeSptr node = NewNode();
	node->self = cur;
	node->PushBackChild(rhs);
	return node;
}


TreeNodeSptr Parser::ParseCompoundStmt(const std::set<EToken>& allowed /* = std::set<EToken>() */)
{
	if(GetCur().kind != EToken::LBrace)
	{
		return nullptr;
	}

	TreeNodeSptr compound = NewNode();
	compound->self = GetCur();
	MoveNext();

	for( ; GetCur().kind != EToken::RBrace; )
	{
		if(IsEnd())
		{
			_errors.push_back(ErrorBuilder::Missing(compound->self.line, '}'));
			return nullptr;
		}

		TreeNodeSptr stmt = ParseStmt(allowed);
		if(!stmt)
		{
			return nullptr;
		}

		compound->PushBackChild(stmt);
	}

	MoveNext();

	return compound;
}

TreeNodeSptr Parser::ParseStmt(const std::set<EToken>& allowed /* = std::set<EToken>() */)
{
	TreeNodeSptr ast;

	if(ast = ParseCompoundStmt(allowed)) return ast;

	if(ast = ParseInclude()) return ast;
	if(ast = ParseIf(allowed)) return ast;
	if(ast = ParseFor(allowed)) return ast;
	if(ast = ParseFn()) return ast;
	if(ast = ParseClass()) return ast;

	if(ast = ParseExpLoop(EToken::Semicolon))
	{
		if(GetCur().kind != EToken::Semicolon)
		{
			_errors.push_back(ErrorBuilder::Missing(GetCur().line, ';'));
			return nullptr;
		}
		MoveNext();
		if(ast) return ast;
	}
	else
	{
		if(GetCur().kind == EToken::Semicolon)
		{
			MoveNext();
			return ParseStmt();
		}
	}

	if(IsEnd()) return nullptr;

	auto cur = GetCur();
	if(allowed.find(cur.kind) == allowed.end())
	{
		//todo correct message
		_errors.push_back(ErrorBuilder::SyntaxError(GetCur().line, "tbd"));
		MoveNext();
		return nullptr;
	}

	if(cur.kind == EToken::Continue || cur.kind == EToken::Break)
	{
		if(GetNext().kind != EToken::Semicolon)
		{
			_errors.push_back(ErrorBuilder::Missing(cur.line, ';'));
			return nullptr;
		}
		ast = NewNode();
		ast->self = cur;
		MoveNext();
		MoveNext();
		return ast;
	}
	else if(cur.kind == EToken::Return)
	{
		ast = NewNode();
		ast->self = cur;
		MoveNext();
		TreeNodeSptr ret = ParseExpLoop();
		if(ret)
		{
			ast->PushBackChild(ret);
		}

		if(GetCur().kind != EToken::Semicolon)
		{
			_errors.push_back(ErrorBuilder::Missing(cur.line, ';'));
			return nullptr;
		}
		MoveNext();

		return ast;
	}

	//todo correct message
	_errors.push_back(ErrorBuilder::SyntaxError(GetCur().line, "tbd"));
	return nullptr;
}

TreeNodeSptr Parser::ParseInclude()
{
	if(GetCur().kind != EToken::Include)
	{
		return nullptr;
	}

	TreeNodeSptr inc = NewNode(GetCur());
	MoveNext();

	if(GetCur().kind != EToken::Id)
	{
		_errors.push_back(ErrorBuilder::Expected(inc->self.line, "INCLUDE_NAME"));
		return nullptr;
	}

	inc->PushBackChild(NewNode(GetCur()));
	MoveNext();
	return inc;
}

TreeNodeSptr Parser::ParseIf(const std::set<EToken>& allowed /* = std::set<EToken>() */)
{
	if(GetCur().kind != EToken::If)
	{
		return nullptr;
	}

	Token _if = GetCur();
	MoveNext();

	if(GetCur().kind != EToken::LParen)
	{
		_errors.push_back(ErrorBuilder::Missing(_if.line, '('));
		return nullptr;
	}
	MoveNext();

	TreeNodeSptr exp = ParseExpLoop(EToken::RParen);
	if(!exp)
	{
		_errors.push_back(ErrorBuilder::SyntaxError(_if.line, ')'));
		return nullptr;
	}
	if(GetCur().kind != EToken::RParen)
	{
		_errors.push_back(ErrorBuilder::Missing(_if.line, ')'));
		return nullptr;
	}
	MoveNext();

	TreeNodeSptr ifNode = NewNode();
	ifNode->self = _if;
	ifNode->PushBackChild(exp);

	TreeNodeSptr _true = ParseStmt(allowed);
	if(!_true)
	{
		_errors.push_back(ErrorBuilder::ExpectedExpression(_if.line, "if"));
		return nullptr;
	}

	ifNode->PushBackChild(_true);

	if(GetCur().kind == EToken::Else)
	{
		uint32_t curLine = GetCur().line;
		MoveNext();
		TreeNodeSptr _false = ParseStmt(allowed);
		if(!_false)
		{
			_errors.push_back(ErrorBuilder::ExpectedExpression(_if.line, "else"));
			return nullptr;
		}

		ifNode->PushBackChild(_false);
	}

	return ifNode;
}

TreeNodeSptr Parser::ParseFor(const std::set<EToken>& allowed /* = std::set<EToken>() */)
{
	if(GetCur().kind != EToken::For)
	{
		return nullptr;
	}

	Token _for = GetCur();
	MoveNext();

	if(GetCur().kind != EToken::LParen)
	{
		_errors.push_back(ErrorBuilder::Missing(_for.line, '('));
		return nullptr;
	}
	MoveNext();

	TreeNodeSptr init = ParseExpLoop(EToken::Semicolon);
	if(!init)
	{
		init = NewNode();
		init->self = { EToken::Int, _for.line, "1" };
	}
	if(GetCur().kind != EToken::Semicolon)
	{
		_errors.push_back(ErrorBuilder::Missing(_for.line, ';'));
		return nullptr;
	}
	MoveNext();

	TreeNodeSptr cond = ParseExpLoop(EToken::Semicolon);
	if(!cond)
	{
		cond = NewNode();
		cond->self = { EToken::Int, _for.line, "1" };
	}
	if(GetCur().kind != EToken::Semicolon)
	{
		_errors.push_back(ErrorBuilder::Missing(_for.line, ';'));
		return nullptr;
	}
	MoveNext();

	TreeNodeSptr update = ParseExpLoop(EToken::RParen);
	if(!update)
	{
		update = NewNode();
		update->self = { EToken::Int, _for.line, "1" };
	}
	if(GetCur().kind != EToken::RParen)
	{
		_errors.push_back(ErrorBuilder::Missing(_for.line, ')'));
		return nullptr;
	}
	MoveNext();

	TreeNodeSptr forNode = NewNode();
	forNode->self = _for;
	forNode->PushBackChild(init);
	forNode->PushBackChild(cond);
	forNode->PushBackChild(update);

	set<EToken> localAllowed = allowed;
	localAllowed.insert(s_allowedFor.begin(), s_allowedFor.end());

	TreeNodeSptr loop = ParseStmt(localAllowed);
	if(!loop)
	{	//todo need message
		return nullptr;
	}

	forNode->PushBackChild(loop);
	return forNode;
}

TreeNodeSptr Parser::ParseFn()
{
	if(GetCur().kind != EToken::Fn)
	{
		return nullptr;
	}

	Token fn = GetCur();
	MoveNext();

	if(GetCur().kind != EToken::Id)
	{
		_errors.push_back(ErrorBuilder::SyntaxError(fn.line, "fn"));
		return nullptr;
	}
	fn.val = GetCur().val;
	MoveNext();

	if(GetCur().kind != EToken::LParen)
	{
		_errors.push_back(ErrorBuilder::Missing(fn.line, '('));
		return nullptr;
	}
	MoveNext();

	TreeNodeSptr params = NewNode();
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
			TreeNodeSptr param = NewNode();
			param->self = GetCur();
			params->PushBackChild(param);
			MoveNext();
			if(GetCur().kind == EToken::Comma)
			{
				MoveNext();
			}
			else if(GetCur().kind != EToken::RParen)
			{//todo leak
				_errors.push_back(ErrorBuilder::Missing(fn.line, ')'));
				return nullptr;
			}
		}
		else
		{//todo leak
		//todo correct message
			_errors.push_back(ErrorBuilder::SyntaxError(fn.line, "fn"));
			return nullptr;
		}
	}

	TreeNodeSptr body = ParseStmt(s_allowedFn);
	if(!body)
	{//todo leak
		//todo correct message
		_errors.push_back(ErrorBuilder::SyntaxError(fn.line, "fn"));
		return nullptr;
	}

	TreeNodeSptr fnNode = NewNode();
	fnNode->self = fn;
	fnNode->PushBackChild(params);
	fnNode->PushBackChild(body);
	return fnNode;
}

TreeNodeSptr Parser::ParseClass()
{
	if(GetCur() != EToken::Class)
	{
		return nullptr;
	}
	MoveNext();

	Token id = GetCur();
	if(id != EToken::Id)
	{
		_errors.push_back(ErrorBuilder::Expected(id.line, "identifier"));
		return nullptr;
	}
	MoveNext();

	TreeNodeSptr cls = NewNode(id);
	cls->self.kind = EToken::Class;

	if(GetCur().kind != EToken::LBrace)
	{
		_errors.push_back(ErrorBuilder::SyntaxError(GetCur().line, GetCur().val));
		return nullptr;
	}
	MoveNext();

	for( ; GetCur().kind != EToken::RBrace; )
	{
		if(IsEnd())
		{
			_errors.push_back(ErrorBuilder::Missing(cls->self.line, '}'));
			return nullptr;
		}

		if(GetCur() == EToken::Semicolon)
		{
			MoveNext();
			continue;
		}

		TreeNodeSptr stmt;
		if(!stmt)
		{
			stmt = ParseFn();
		}
		if(!stmt)
		{
			if(stmt = ParseExpLoop(EToken::Semicolon))
			{
				if(stmt->self != EToken::Assign)
				{
					_errors.push_back(ErrorBuilder::SyntaxError(stmt->self.line, stmt->self.val));
					return nullptr;
				}

				if(GetCur().kind != EToken::Semicolon)
				{
					_errors.push_back(ErrorBuilder::Missing(GetCur().line, GetCur().val));
					return nullptr;
				}
				MoveNext();
			}
		}
		if(!stmt)
		{
			_errors.push_back(ErrorBuilder::SyntaxError(GetCur().line, GetCur().val));
			return nullptr;
		}

		cls->PushBackChild(stmt);
	}
	MoveNext();

	return cls;
}


TreeNodeSptr Parser::Parse()
{
	TreeNodeSptr root = NewNode();

	for( ; !IsEnd(); )
	{
		if(GetCur() == EToken::Semicolon)
		{
			MoveNext();
			continue;
		}

		TreeNodeSptr ast = ParseStmt();
		if(!ast)
		{
			return nullptr;
		}
		root->PushBackChild(ast);
	}
	if(!_errors.empty()) return nullptr;
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
