#pragma once
#include "Token.h"
#include <vector>
#include <memory>


struct TreeNode;
using TreeNodeSptr = std::shared_ptr<TreeNode>;

struct TreeNode
{
	Token self;
	int priority = 0;
	TreeNode* parent = nullptr;
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


	inline void PopChild(TreeNodeSptr node)
	{
		for(auto it = childs.begin(); it != childs.end(); it++)
		{
			if(*it == node)
			{
				node->parent = nullptr;
				childs.erase(it);
				break;
			}
		}
	}

	inline bool ReplaceChild(TreeNodeSptr oldChild, TreeNodeSptr newChild)
	{
		for(auto it = childs.begin(); it != childs.end(); it++)
		{
			if(*it == oldChild)
			{
				newChild->parent = oldChild->parent;
				oldChild->parent = nullptr;
				*it = newChild;
				return true;
			}
		}
		return false;
	}

	inline bool ReplaceFrontChild(TreeNodeSptr node)
	{
		if(childs.empty())
			return false;

		childs.front()->parent = nullptr;
		childs.front() = node;
		node->parent = this;
		return true;
	}

	inline bool ReplaceBackChild(TreeNodeSptr node)
	{
		if(childs.empty())
			return false;

		childs.back()->parent = nullptr;
		childs.back() = node;
		node->parent = this;
		return true;
	}

	static TreeNodeSptr New(const Token& tok = Token())
	{
		auto tn = std::make_shared<TreeNode>();
		tn->self = tok;
		return tn;
	}
};
