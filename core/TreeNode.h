#pragma once
#include "Token.h"
#include <vector>
#include <memory>


struct TreeNode;
using TreeNodeSptr = std::shared_ptr<TreeNode>;

struct TreeNode
{
	Token self;
	TreeNodeSptr parent = nullptr;
	std::vector<TreeNodeSptr> childs;

	static TreeNodeSptr New()
	{
		return std::make_shared<TreeNode>();
	}
};
