#include "File.h"
#include "vm/Variable.h"
#include <stdio.h>


namespace ybuiltin::File
{
using namespace yvm;
using namespace ymod;


YRet Open(YArgs* args)
{
	Variable* me = nullptr;
	int argStart = 0;
	if(args->numArgs > 2)
	{
		me = (Variable*)args->args[0].o;
		argStart = 1;
	}

	FILE* fp = fopen(((Variable*)args->args[argStart].o)->str().c_str(), ((Variable*)args->args[argStart+1].o)->str().c_str());
	if(fp == nullptr)
		return { errno, };

	if(me)
	{//TODO temp...
		me->modObj()._o = fp;
		return {};
	}
	else
	{
		YRet yr;
		YObj* yo = new YObj;
		yr.single.tp = YEArg::Object;
		yo->name.SetStr(GetModuleDesc().name);
		yo->obj = fp;
		yr.single.o = yo;
		return yr;
	}
}

YRet Close(YArgs* args)
{
	Variable* me = (Variable*)args->args[0].o;
	FILE* fp = (FILE*)me->modObj()._o;
	int ec = fclose(fp) ? errno : 0;
	if(!ec) me->modObj()._o = nullptr;
	return { ec };
}

YRet Read(YArgs* args)
{
	Variable* me = (Variable*)args->args[0].o;
	Variable* sz = (Variable*)args->args[1].o;

	std::string s;
	s.resize(sz->int_() + 1);
	size_t rsz = fread(s.data(), 1, sz->int_(), (FILE*)me->modObj()._o);
	s.resize(rsz);
	YRet yr;
	auto rv = (Variable*)args->retBuff.o;
	rv->SetStr(s);
	yr.single.SetYVar(rv);
	return yr;
}

YRet Write(YArgs* args)
{
	Variable* me = (Variable*)args->args[0].o;
	Variable* s = (Variable*)args->args[1].o;

	size_t wsz = 0;
	if(*s == Variable::STR)
	{
		wsz = fwrite(s->str().c_str(), 1, s->str().size(), (FILE*)me->modObj()._o);
	}
	else
	{//TODO
		//yet;
		throw 'n';
	}

	int code = 0;
	if(wsz < s->str().size()) code = errno ? errno : 1;
	return { code };
}

const ModuleDesc& GetModuleDesc()
{
	static ModuleDesc m;

	if(m.name.empty())
	{
		m.name = "file";
		m.builtin = true;
		m.memberTbl["open"] = ModuleMemberDesc{ "open", ModuleMemberDesc::FUNC, false, 2, Open };
		m.memberTbl["close"] = ModuleMemberDesc{ "close", ModuleMemberDesc::FUNC, true, 0, Close };
		m.memberTbl["read"] = ModuleMemberDesc{ "read", ModuleMemberDesc::FUNC, true, 1, Read };
		m.memberTbl["write"] = ModuleMemberDesc{ "write", ModuleMemberDesc::FUNC, true, 1, Write };
	}
	return m;
}

}
