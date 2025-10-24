#pragma once
#include "TreeNode.h"


class SemanticAnalyzer
{
public:
	SemanticAnalyzer();
	~SemanticAnalyzer();

	bool Analyze(const TreeNode& code);

protected:
};
