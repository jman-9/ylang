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
	friend bool InitParser();
	static std::map<EToken, int> _opMap;
	static std::map<EToken, int> _precMap;

	static bool IsOperator(EToken tok);
	static bool IsOperator(const Token& tok);

	static int CompPrec(EToken lhs, EToken rhs);
	static int CompPrec(const Token& lhs, const Token& rhs);

	const std::vector<Token>& _tokens;
	size_t _pos;

	TreeNode* ParseExpLoop(EToken endToken = EToken::None);
	TreeNode* ParseExp(bool first);
	TreeNode* ParsePrimaryExp();
	TreeNode* ParsePrefixExp();
	TreeNode* ParseOpExp();
	TreeNode* ParseCompoundStmt();
	TreeNode* ParseStmt();
	TreeNode* ParseIf();
	TreeNode* ParseFor();
	TreeNode* ParseFn();

	bool MoveNext();
	bool MovePrev();
	bool IsEnd() const;
	const Token& GetPrev() const;
	const Token& GetCur() const;
};
