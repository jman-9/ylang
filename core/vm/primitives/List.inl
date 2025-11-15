#pragma once
#include "List.h"
#include "vm/Variable.h"


namespace yvm::primitive::List
{
using namespace yvm;
using namespace ymod;
using namespace std;


inline YRet Len(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = Variable::NewNum((int64_t)self->_list->size());
	return {};
}

inline YRet Append(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	auto a = (Variable*)args->args[1].o;

	self->_list->push_back(a->Clone());
	return {};
}

inline YRet Insert(YArgs* args)
{
	if(args->numArgs < 3)
		throw 'n';//TODO

	auto self = (Variable*)args->args[0].o;
	auto i = (Variable*)args->args[1].o;
	auto v =(Variable*)args->args[2].o;

	self->_list->insert(self->_list->begin() + i->_int, v->Clone());
	return {};
}

inline YRet Pop(YArgs* args)
{
	if(args->numArgs < 2)
		throw 'n';//TODO

	auto self = (Variable*)args->args[0].o;
	auto i = (Variable*)args->args[1].o;


	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = self->_list->at(i->_int);
	self->_list->erase(self->_list->begin() + i->_int);
	return yr;
}

inline YRet PopFront(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = self->_list->front();
	self->_list->erase(self->_list->begin());
	return yr;
}

inline YRet PopBack(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = self->_list->back();
	self->_list->pop_back();
	return yr;
}

const ymod::Module& GetModule()
{
	static Module m;
	if(m.name.empty())
	{
		m.name = "list";
		m.builtin = true;
		m.funcTbl[ "len" ] = { "len", true, 0, Len };
		m.funcTbl[ "append" ] = { "append", true, 1, Append };
		m.funcTbl[ "insert" ] = { "insert", true, 2, Insert };
		m.funcTbl[ "pop" ] = { "pop", true, 1, Pop };
		m.funcTbl[ "pop_front" ] = { "pop_front", true, 0, PopFront };
		m.funcTbl[ "pop_back" ] = { "pop_back", true, 0, PopBack };
	}
	return m;
}

}
