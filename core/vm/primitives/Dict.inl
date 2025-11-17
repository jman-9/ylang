#pragma once
#include "Dict.h"
#include "vm/Variable.h"


namespace yvm::primitive::Dict
{
using namespace yvm;
using namespace ymod;
using namespace std;


inline YRet Len(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = Variable::NewInt((int64_t)self->_dict->size());
	return {};
}

inline YRet Contains(YArgs* args)
{
	if(args->numArgs < 2)
		throw 'n';//TODO

	auto self = (Variable*)args->args[0].o;
	auto k = (Variable*)args->args[1].o;

	auto found = self->_dict->find(k->_str);
	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = Variable::NewInt( found != self->_dict->end() );
	return yr;
}

inline YRet Keys(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	auto ret = Variable::NewList();
	for(auto& [k, _] : *self->_dict)
	{
		ret->_list->push_back(Variable::NewStr(k));
	}

	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = ret;
	return yr;
}

inline YRet Values(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	auto ret = Variable::NewList();
	for(auto& [_, v] : *self->_dict)
	{
		ret->_list->push_back(v);
	}

	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = ret;
	return yr;
}

inline YRet Items(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	auto ret = Variable::NewList();
	for(auto& [k, v] : *self->_dict)
	{
		Variable* pair = Variable::NewList();
		Variable* kv = Variable::NewStr(k);
		pair->_list->push_back(kv);
		pair->_list->push_back(v);

		ret->_list->push_back(pair);
	}

	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = ret;
	return yr;
}

inline YRet Pop(YArgs* args)
{
	if(args->numArgs < 2)
		throw 'n';//TODO

	auto self = (Variable*)args->args[0].o;
	auto k = (Variable*)args->args[1].o;

	auto found = self->_dict->find(k->_str);
	Variable* ret = nullptr;
	if(found != self->_dict->end())
	{
		ret = found->second;
		self->_dict->erase(found);
	}

	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = ret;
	return yr;
}

const ymod::ModuleDesc& GetModuleDesc()
{
	static ModuleDesc m;
	if(m.name.empty())
	{
		m.name = "dict";
		m.builtin = true;
		m.memberTbl[ "len" ] = { "len", ymod::ModuleMemberDesc::FUNC, true, 0, Len };
		m.memberTbl[ "contains" ] = { "contains", ymod::ModuleMemberDesc::FUNC, true, 1, Contains };
		m.memberTbl[ "keys" ] = { "keys", ymod::ModuleMemberDesc::FUNC, true, 0, Keys };
		m.memberTbl[ "values" ] = { "values", ymod::ModuleMemberDesc::FUNC, true, 0, Values };
		m.memberTbl[ "items" ] = { "items", ymod::ModuleMemberDesc::FUNC, true, 0, Items };
		m.memberTbl[ "pop" ] = { "pop", ymod::ModuleMemberDesc::FUNC, true, 1, Pop };
	}
	return m;
}

}
