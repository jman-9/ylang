#pragma once
#include <stdint.h>
#include <vector>

namespace yvm
{
struct Variable;

class VariableList
{
	static uint32_t constexpr _initialSize = 3;
	static uint32_t constexpr _growSize = 33;

public:
	VariableList()
	{
		Get(_initialSize-1);
	}
	~VariableList();

	inline Variable* Get(uint32_t idx)
	{
		if(idx >= _list.size())
		{
			GrowList(idx + _growSize);
		}
		return _list[idx];
	}

private:
	void GrowList(uint32_t growSize);
	std::vector<Variable*> _list;
};

}
