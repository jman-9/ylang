#pragma once
#include "TreeNode.h"
#include "Error.h"
#include <vector>
#include <map>
#include <set>


namespace ycom
{

class Parser
{
public:
	Parser(const std::vector<Token>& tokens);
	virtual ~Parser();

	TreeNodeSptr Parse();
	std::vector<Error> _errors;

protected:
	const std::vector<Token>& _tokens;
	size_t _pos;

	TreeNodeSptr ParseExpLoop(EToken endToken = EToken::None, EToken endToken2 = EToken::None);
	TreeNodeSptr ParseExp(bool first);
	TreeNodeSptr ParsePrimaryExp();
	TreeNodeSptr ParsePostfixExp();
	TreeNodeSptr ParsePrefixExp();
	TreeNodeSptr ParseOpExp();
	TreeNodeSptr ParseCompoundStmt();
	TreeNodeSptr ParseStmt();
	TreeNodeSptr ParseInclude();
	TreeNodeSptr ParseIf();
	TreeNodeSptr ParseFor();
	TreeNodeSptr ParseFn();
	TreeNodeSptr ParseClass();

	bool MoveNext();
	bool MovePrev();
	bool IsEnd() const;
	const Token& GetPrev() const;
	const Token& GetCur() const;
	const Token& GetNext() const;
};

}
