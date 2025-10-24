#pragma once
#include "Token.h"
#include <vector>
#include <map>


struct TreeNode
{
	Token self;
	TreeNode* parent = nullptr;
	std::vector<TreeNode*> childs;
};

class Parser
{
public:
	Parser(const std::vector<Token>& tokens);
	virtual ~Parser();

	TreeNode* Parse();

protected:
	const std::vector<Token>& _tokens;
	size_t _pos;

	TreeNode* ParseExpLoop(EToken endToken = EToken::None, EToken endToken2 = EToken::None);
	TreeNode* ParseExp(bool first);
	TreeNode* ParsePrimaryExp();
	TreeNode* ParsePostfixExp();
	TreeNode* ParsePrefixExp();
	TreeNode* ParseOpExp();
	TreeNode* ParseCompoundStmt(const std::vector<EToken>& allowed = std::vector<EToken>());
	TreeNode* ParseStmt(const std::vector<EToken>& allowed = std::vector<EToken>());
	TreeNode* ParseIf();
	TreeNode* ParseFor();
	TreeNode* ParseFn();

	bool MoveNext();
	bool MovePrev();
	bool IsEnd() const;
	const Token& GetPrev() const;
	const Token& GetCur() const;
	const Token& GetNext() const;
};
