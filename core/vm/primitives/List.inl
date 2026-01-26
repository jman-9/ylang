#pragma once
#include "List.h"
#include "vm/Variable.h"
#include "vm/RuntimeError.h"
#include "module/ModuleUtil.h"


namespace yvm::primitive::List
{
using namespace yvm;
using namespace ymod;
using namespace std;


inline YRet Empty(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	auto v = (Variable*)args->retBuff.o;
	v->SetBool(self->list().empty());
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet Size(YArgs* args)
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

inline YRet PushBack(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	auto a = (Variable*)args->args[1].o;

	self->list().push_back(*a);
	return {};
}

inline YRet Insert(YArgs* args)
{
	if(args->numArgs < 3)
		INTERNALERR_NUMARGS(3, args->numArgs);

	auto self = (Variable*)args->args[0].o;
	MODARG_VAR(1, i, Variable::INT);
	auto v = (Variable*)args->args[2].o;

	if(i.int_() < 0 || i.int_() >= self->bytes().size())
	{//qaz TODO
		throw RuntimeError::OutOfRange(self->_type, "bytes", i.int_(), self->bytes().size());
	}

	self->list().insert(self->list().begin() + i.int_(), *v);
	return {};
}

inline YRet Pop(YArgs* args)
{
	if(args->numArgs < 2)
		INTERNALERR_NUMARGS(2, args->numArgs);

	auto self = (Variable*)args->args[0].o;
	MODARG_VAR(1, idx, Variable::INT);

	if(idx.int_() < 0 || idx.int_() >= self->bytes().size())
	{//qaz TODO
		throw RuntimeError::OutOfRange(self->_type, "bytes", idx.int_(), self->bytes().size());
	}

	Variable popped = self->list()[idx.int_()];
	self->list().erase(self->list().begin() + idx.int_());

	auto v = (Variable*)args->retBuff.o;
	v->SetVar(popped);
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet PopFront(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	if(self->list().empty())
	{//qaz TODO
		throw RuntimeError::OutOfRange(self->_type, "list", 0, 0);
	}

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

	if(self->list().empty())
	{//qaz TODO
		throw RuntimeError::OutOfRange(self->_type, "list", 0, 0);
	}

	Variable popped = self->list().back();
	self->list().pop_back();

	auto v = (Variable*)args->retBuff.o;
	v->SetVar(popped);
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet Front(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	if(self->list().empty())
	{//qaz TODO
		throw RuntimeError::OutOfRange(self->_type, "list", 0, 0);
	}

	auto v = (Variable*)args->retBuff.o;
	v->SetVar(self->list().front());
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet Back(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	if(self->list().empty())
	{//qaz TODO
		throw RuntimeError::OutOfRange(self->_type, "list", 0, 0);
	}

	auto v = (Variable*)args->retBuff.o;
	v->SetVar(self->list().back());
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
		m.memberTbl[ "empty" ] = { "empty", ymod::ModuleMemberDesc::FUNC, true, 0, Empty };
		m.memberTbl[ "size" ] = { "size", ymod::ModuleMemberDesc::FUNC, true, 0, Size };
		m.memberTbl[ "resize" ] = { "resize", ymod::ModuleMemberDesc::FUNC, true, 1, Resize };
		m.memberTbl[ "push_back" ] = { "push_back", ymod::ModuleMemberDesc::FUNC, true, 1, PushBack };
		m.memberTbl[ "insert" ] = { "insert", ymod::ModuleMemberDesc::FUNC, true, 2, Insert };
		m.memberTbl[ "pop" ] = { "pop", ymod::ModuleMemberDesc::FUNC, true, 1, Pop };
		m.memberTbl[ "pop_front" ] = { "pop_front", ymod::ModuleMemberDesc::FUNC, true, 0, PopFront };
		m.memberTbl[ "pop_back" ] = { "pop_back", ymod::ModuleMemberDesc::FUNC, true, 0, PopBack };
		m.memberTbl[ "front" ] = { "front", ymod::ModuleMemberDesc::FUNC, true, 0, Front };
		m.memberTbl[ "back" ] = { "back", ymod::ModuleMemberDesc::FUNC, true, 0, Back };
	}
	return m;
}

}
