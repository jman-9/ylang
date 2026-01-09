#pragma once
#include "Dict.h"
#include "vm/Variable.h"
#include "vm/RuntimeError.h"


namespace yvm::primitive::Dict
{
using namespace yvm;
using namespace ymod;
using namespace std;


inline YRet Empty(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	auto v = (Variable*)args->retBuff.o;
	v->SetBool(self->dict().empty());
	YRet yr;
	yr.single.SetYVar(v);
	return yr;
}

inline YRet Size(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	YRet yr;
	auto v = (Variable*)args->retBuff.o;
	v->SetInt((int64_t)self->dict().size());
	yr.single.SetYVar(v);
	return {};
}

inline YRet Contains(YArgs* args)
{
	if(args->numArgs < 2)
		INTERNALERR_NUMARGS(2, args->numArgs);

	auto self = (Variable*)args->args[0].o;
	auto k = (Variable*)args->args[1].o;

	auto found = self->dict().find(k->str());

	YRet yr;
	auto v = (Variable*)args->retBuff.o;
	v->SetInt(found != self->dict().end());
	yr.single.SetYVar(v);
	return yr;
}

inline YRet Keys(YArgs* args)
{
	auto self = *(Variable*)args->args[0].o;

	auto ret = (Variable*)args->retBuff.o;
	ret->SetList();
	for(auto& [k, _] : self.dict())
	{
		ret->list().push_back({});
		ret->list().back().SetStr(k);
	}

	YRet yr;
	yr.single.SetYVar(ret);
	return yr;
}

inline YRet Values(YArgs* args)
{
	auto self = *(Variable*)args->args[0].o;

	auto ret = (Variable*)args->retBuff.o;
	ret->SetList();
	for(auto& [_, v] : self.dict())
	{
		ret->list().push_back(v);
	}

	YRet yr;
	yr.single.SetYVar(ret);
	return yr;
}

inline YRet Items(YArgs* args)
{
	auto self = *(Variable*)args->args[0].o;

	auto ret = (Variable*)args->retBuff.o;
	ret->SetList();
	for(auto& [k, v] : self.dict())
	{
		ret->list().push_back({});
		auto& pair = ret->list().back();
		pair.SetList();
		pair.list().push_back({});
		pair.list().back().SetStr(k);
		pair.list().push_back(v);
	}

	YRet yr;
	yr.single.SetYVar(ret);
	return yr;
}

inline YRet Pop(YArgs* args)
{
	if(args->numArgs < 2)
		INTERNALERR_NUMARGS(2, args->numArgs);

	auto self = *(Variable*)args->args[0].o;
	auto k = (Variable*)args->args[1].o;

	auto found = self.dict().find(k->str());
	auto ret = (Variable*)args->retBuff.o;
	if(found != self.dict().end())
	{
		ret->SetVar(found->second);
		self.dict().erase(found);
	}

	YRet yr;
	yr.single.SetYVar(ret);
	return yr;
}

const ymod::ModuleDesc& GetModuleDesc()
{
	static ModuleDesc m;
	if(m.name.empty())
	{
		m.name = "dict";
		m.builtin = true;
		m.memberTbl[ "empty" ] = { "empty", ymod::ModuleMemberDesc::FUNC, true, 0, Empty };
		m.memberTbl[ "size" ] = { "len", ymod::ModuleMemberDesc::FUNC, true, 0, Size };
		m.memberTbl[ "contains" ] = { "contains", ymod::ModuleMemberDesc::FUNC, true, 1, Contains };
		m.memberTbl[ "keys" ] = { "keys", ymod::ModuleMemberDesc::FUNC, true, 0, Keys };
		m.memberTbl[ "values" ] = { "values", ymod::ModuleMemberDesc::FUNC, true, 0, Values };
		m.memberTbl[ "items" ] = { "items", ymod::ModuleMemberDesc::FUNC, true, 0, Items };
		m.memberTbl[ "pop" ] = { "pop", ymod::ModuleMemberDesc::FUNC, true, 1, Pop };
	}
	return m;
}

}
