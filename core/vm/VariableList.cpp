#include "VariableList.h"
#include "Variable.h"


namespace yvm
{

VariableList::~VariableList()
{
	for(Variable* e : _list)
		delete e;
}

void VariableList::GrowList(uint32_t growSize)
{
	size_t oldSize = _list.size();
	_list.resize(_list.size() + growSize);
	for(size_t i=oldSize; i<_list.size(); i++)
	{
		_list[i] = new Variable();
	}
}

}
