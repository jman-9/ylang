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

	FILE* fp = fopen(((Variable*)args->args[argStart].o)->_str.c_str(), ((Variable*)args->args[argStart+1].o)->_str.c_str());
	if(fp == nullptr)
		return { errno, };

	if(me)
	{
		me->_obj = fp;
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
	FILE* fp = (FILE*)me->_obj;
	int ec = fclose(fp) ? errno : 0;
	return { ec };
}

YRet Read(YArgs* args)
{
	Variable* me = (Variable*)args->args[0].o;
	Variable* sz = (Variable*)args->args[1].o;

	std::string s;
	s.resize(sz->_int + 1);
	size_t rsz = fread(s.data(), 1, sz->_int, (FILE*)me->_obj);
	s.resize(rsz);
	YRet yr;
	yr.single.SetYVar(Variable::NewStr(s));
	return yr;
}

YRet Write(YArgs* args)
{
	Variable* me = (Variable*)args->args[0].o;
	Variable* s = (Variable*)args->args[1].o;

	size_t wsz = 0;
	if(*s == Variable::STR)
	{
		wsz = fwrite(s->_str.c_str(), 1, s->_str.size(), (FILE*)me->_obj);
	}
	else
	{//TODO
		//yet;
		throw 'n';
	}

	int code = 0;
	if(wsz < s->_str.size()) code = errno ? errno : 1;
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
		m.memberTbl["write"] = ModuleMemberDesc{ "read", ModuleMemberDesc::FUNC, true, 1, Write };
	}
	return m;
}

}
