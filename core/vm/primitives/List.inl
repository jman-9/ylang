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

	auto v = (Variable*)args->retBuff.o;
	v->SetInt((int64_t)self->list().size());
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet Resize(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	auto n = (Variable*)args->args[1].o;

	self->list().resize(n->int_());
	return {};
}

inline YRet Append(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	auto a = (Variable*)args->args[1].o;

	self->list().push_back(*a);
	return {};
}

inline YRet Insert(YArgs* args)
{
	if(args->numArgs < 3)
		throw 'n';//TODO

	auto self = (Variable*)args->args[0].o;
	auto i = (Variable*)args->args[1].o;
	auto v =(Variable*)args->args[2].o;

	self->list().insert(self->list().begin() + i->int_(), *v);
	return {};
}

inline YRet Pop(YArgs* args)
{
	if(args->numArgs < 2)
		throw 'n';//TODO

	auto self = (Variable*)args->args[0].o;
	auto i = (Variable*)args->args[1].o;

	Variable popped = self->list()[i->int_()];
	self->list().erase(self->list().begin() + i->int_());

	auto v = (Variable*)args->retBuff.o;
	v->SetVar(popped);
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet PopFront(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	Variable popped = self->list().front();
	self->list().erase(self->list().begin());

	auto v = (Variable*)args->retBuff.o;
	v->SetVar(popped);
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet PopBack(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	Variable popped = self->list().back();
	self->list().pop_back();

	auto v = (Variable*)args->retBuff.o;
	v->SetVar(popped);
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

const ymod::ModuleDesc& GetModuleDesc()
{
	static ModuleDesc m;
	if(m.name.empty())
	{
		m.name = "list";
		m.builtin = true;
		m.memberTbl[ "len" ] = { "len", ymod::ModuleMemberDesc::FUNC, true, 0, Len };
		m.memberTbl[ "resize" ] = { "resize", ymod::ModuleMemberDesc::FUNC, true, 1, Resize };
		m.memberTbl[ "append" ] = { "append", ymod::ModuleMemberDesc::FUNC, true, 1, Append };
		m.memberTbl[ "insert" ] = { "insert", ymod::ModuleMemberDesc::FUNC, true, 2, Insert };
		m.memberTbl[ "pop" ] = { "pop", ymod::ModuleMemberDesc::FUNC, true, 1, Pop };
		m.memberTbl[ "pop_front" ] = { "pop_front", ymod::ModuleMemberDesc::FUNC, true, 0, PopFront };
		m.memberTbl[ "pop_back" ] = { "pop_back", ymod::ModuleMemberDesc::FUNC, true, 0, PopBack };
	}
	return m;
}

}
