#pragma once
#include "TreeNode.h"
#include "Error.h"
#include <vector>
#include <map>
#include <set>


class Parser
{
public:
	Parser(const std::vector<Token>& tokens);
	virtual ~Parser();

	TreeNode* Parse();
	std::vector<Error> _errors;

protected:
	const std::vector<Token>& _tokens;
	size_t _pos;

	TreeNode* ParseExpLoop(EToken endToken = EToken::None, EToken endToken2 = EToken::None);
	TreeNode* ParseExp(bool first);
	TreeNode* ParsePrimaryExp();
	TreeNode* ParsePostfixExp();
	TreeNode* ParsePrefixExp();
	TreeNode* ParseOpExp();
	TreeNode* ParseDictExp();
	TreeNode* ParseCompoundStmt(const std::set<EToken>& allowed = std::set<EToken>());
	TreeNode* ParseStmt(const std::set<EToken>& allowed = std::set<EToken>());
	TreeNode* ParseIf(const std::set<EToken>& allowed = std::set<EToken>());
	TreeNode* ParseFor(const std::set<EToken>& allowed = std::set<EToken>());
	TreeNode* ParseFn();

	bool MoveNext();
	bool MovePrev();
	bool IsEnd() const;
	const Token& GetPrev() const;
	const Token& GetCur() const;
	const Token& GetNext() const;
};
