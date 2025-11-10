#pragma once
#include "Token.h"
#include <vector>
#include <memory>


struct TreeNode;
using TreeNodeSptr = std::shared_ptr<TreeNode>;

struct TreeNode
{
	Token self;
	TreeNode* parent;
	std::vector<TreeNodeSptr> childs;

	inline void PushFrontChild(TreeNodeSptr node)
	{
		childs.insert(childs.begin(), 1, node);
		node->parent = this;
	}

	inline void PushBackChild(TreeNodeSptr node)
	{
		childs.push_back(node);
		node->parent = this;
	}

	inline void PopFrontChild()
	{
		if(childs.empty()) return;

		childs.front()->parent = nullptr;
		childs.erase(childs.begin());
	}

	inline void PopBackChild()
	{
		if(childs.empty()) return;

		childs.back()->parent = nullptr;
		childs.pop_back();
	}

	inline void ReplaceFrontChild(TreeNodeSptr node)
	{
		if(childs.empty())
		{//TODO
			throw 'n';
		}

		childs.front()->parent = nullptr;
		childs.front() = node;
		node->parent = this;
	}

	inline void ReplaceBackChild(TreeNodeSptr node)
	{
		if(childs.empty())
		{//TODO
			throw 'n';
		}

		childs.back()->parent = nullptr;
		childs.back() = node;
		node->parent = this;
	}

	static TreeNodeSptr New()
	{
		return std::make_shared<TreeNode>();
	}
};
