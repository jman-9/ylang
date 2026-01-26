#pragma once
#include "Bytes.h"
#include "vm/Variable.h"
#include "vm/RuntimeError.h"
#include "module/ModuleUtil.h"


namespace yvm::primitive::Bytes
{
using namespace yvm;
using namespace ymod;
using namespace std;


inline YRet Empty(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	auto v = (Variable*)args->retBuff.o;
	v->SetBool(self->bytes().empty());
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet Size(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	auto v = (Variable*)args->retBuff.o;
	v->SetInt((int64_t)self->bytes().size());
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet Resize(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	MODARG_VAR(1, n, Variable::INT);

	self->bytes().resize(n.int_());
	return {};
}

inline YRet PushBack(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	MODARG_VAR(1, v, Variable::INT);

	if(v.int_() < 0 || v.int_() > 255)
	{//qaz TODO
		throw RuntimeError::OutOfRangeValue(self->_type, "bytes_value", v.int_(), 0, 255);
	}

	self->bytes().push_back((uint8_t)v.int_());
	return {};
}

inline YRet Insert(YArgs* args)
{
	if(args->numArgs < 3)
		INTERNALERR_NUMARGS(3, args->numArgs);

	auto self = (Variable*)args->args[0].o;
	MODARG_VAR(1, i, Variable::INT);
	MODARG_VAR(2, v, Variable::INT);

	if(i.int_() < 0 || i.int_() >= self->bytes().size())
	{//qaz TODO
		throw RuntimeError::OutOfRange(self->_type, "bytes", i.int_(), self->bytes().size());
	}

	if(v.int_() < 0 || v.int_() > 255)
	{//qaz TODO
		throw RuntimeError::OutOfRangeValue(self->_type, "bytes_value", v.int_(), 0, 255);
	}

	self->bytes().insert(self->bytes().begin() + i.int_(), v.int_());
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

	uint8_t popped = self->bytes()[idx.int_()];
	self->bytes().erase(self->bytes().begin() + idx.int_());

	auto v = (Variable*)args->retBuff.o;
	v->SetInt(popped);
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet PopFront(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	if(self->bytes().empty())
	{//qaz TODO
		throw RuntimeError::OutOfRange(self->_type, "bytes", 0, 0);
	}

	uint8_t popped = self->bytes().front();
	self->bytes().erase(self->bytes().begin());

	auto v = (Variable*)args->retBuff.o;
	v->SetInt(popped);
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet PopBack(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	if(self->bytes().empty())
	{//qaz TODO
		throw RuntimeError::OutOfRange(self->_type, "bytes", 0, 0);
	}

	uint8_t popped = self->bytes().back();
	self->bytes().pop_back();

	auto v = (Variable*)args->retBuff.o;
	v->SetInt(popped);
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet Front(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	if(self->bytes().empty())
	{//qaz TODO
		throw RuntimeError::OutOfRange(self->_type, "bytes", 0, 0);
	}

	auto v = (Variable*)args->retBuff.o;
	v->SetInt(self->bytes().front());
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet Back(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	if(self->bytes().empty())
	{//qaz TODO
		throw RuntimeError::OutOfRange(self->_type, "bytes", 0, 0);
	}

	auto v = (Variable*)args->retBuff.o;
	v->SetInt(self->bytes().back());
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet Get(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	MODARG_VAR(1, idx, Variable::INT);

	if(idx.int_() < 0 || idx.int_() >= self->bytes().size())
	{//qaz TODO
		throw RuntimeError::OutOfRange(self->_type, "bytes", idx.int_(), self->bytes().size());
	}

	auto v = (Variable*)args->retBuff.o;
	v->SetInt((uint64_t)self->bytes().at(idx.int_()));
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet Set(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	MODARG_VAR(1, idx, Variable::INT);
	MODARG_VAR(2, v, Variable::INT);

	if(idx.int_() < 0 || idx.int_() >= self->bytes().size())
	{//qaz TODO
		throw RuntimeError::OutOfRange(self->_type, "bytes", idx.int_(), self->bytes().size());
	}
	if(v.int_() < 0 || v.int_() > 255)
	{//qaz TODO
		throw RuntimeError::OutOfRangeValue(self->_type, "bytes_value", v.int_(), 0, 255);
	}

	self->bytes().at(idx.int_()) = (uint8_t)v.int_();
	return {};
}

inline YRet Copy(YArgs* args)
{
	auto& dst = *(Variable*)args->args[0].o;
	MODARG_VAR(1, src, Variable::BYTES);

	int64_t srcStart = 0;
	int64_t copySz = (int64_t)src.bytes().size();
	int64_t dstStart = 0;
	if(args->numArgs > 2)
	{
		MODARG_VAR(2, vsrcStart, Variable::INT);
		srcStart = vsrcStart.int_();

		if(srcStart < 0 || srcStart >= src.bytes().size())
		{//qaz TODO
			throw RuntimeError::OutOfRange(src._type, "bytes", srcStart, src.bytes().size());
		}
		copySz = src.bytes().size() - srcStart;
	}
	if(args->numArgs > 3)
	{
		MODARG_VAR(3, vcopySz, Variable::INT);
		copySz = vcopySz.int_();

		if(copySz < 0 || srcStart + copySz > src.bytes().size())
		{//qaz TODO
			throw RuntimeError::OutOfRange(src._type, "bytes", srcStart + copySz, src.bytes().size());
		}
	}
	if(args->numArgs > 4)
	{
		MODARG_VAR(4, vdstStart, Variable::INT);
		dstStart = vdstStart.int_();

		if(dstStart < 0)
		{//qaz TODO
			throw RuntimeError::OutOfRange(dst._type, "bytes", dstStart, 0);
		}
	}

	if((int64_t)dst.bytes().size() < dstStart + copySz)
	{
		dst.bytes().resize(dstStart + copySz);
	}

	memcpy(dst.bytes().data() + dstStart, src.bytes().data() + srcStart, copySz);
	return {};
}


const ymod::ModuleDesc& GetModuleDesc()
{
	static ModuleDesc m;
	if(m.name.empty())
	{
		m.name = "bytes";
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
		m.memberTbl[ "get" ] = { "get", ymod::ModuleMemberDesc::FUNC, true, 1, Get };
		m.memberTbl[ "set" ] = { "set", ymod::ModuleMemberDesc::FUNC, true, 2, Set };
		m.memberTbl[ "copy" ] = { "copy", ymod::ModuleMemberDesc::FUNC, true, 1, Copy };
	}
	return m;
}

}
