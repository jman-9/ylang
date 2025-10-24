#pragma once
#include "Token.h"
#include <vector>

struct TreeNode
{
	Token self;
	TreeNode* parent = nullptr;
	std::vector<TreeNode*> childs;
};
