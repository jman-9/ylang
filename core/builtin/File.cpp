#include "File.h"
#include "vm/Variable.h"
#include "vm/RuntimeError.h"
#include "module/ModuleUtil.h"
#include <stdio.h>
#include <filesystem>
using namespace std;


namespace ybuiltin::File
{
using namespace yvm;
using namespace ymod;


struct FileObj
{
	FILE* fp = nullptr;
	bool binary = false;
};


YRet Open(YArgs* args)
{
	Variable* me = nullptr;
	const string* path = nullptr;
	const string* mode = nullptr;

	if(args->numArgs > 2)
	{
		MODARG_VAR2(0, vme, Variable::MODULE, Variable::MODULEOBJ);
		me = (Variable*)&vme;
		MODARG_VAR(1, vpath, Variable::STR);
		path = &vpath.str();
		MODARG_VAR(2, vmode, Variable::STR);
		mode = &vmode.str();
	}
	else
	{
		MODARG_VAR(0, vpath, Variable::STR);
		path = &vpath.str();
		MODARG_VAR(1, vmode, Variable::STR);
		mode = &vmode.str();
	}

	FILE* fp = fopen(path->c_str(), mode->c_str());
	if(fp == nullptr)
		return { errno, };

	FileObj* fo = new FileObj{.fp = fp, .binary = mode->find("b") != string::npos };

	if(me)
	{
		me->modObj()._o = fo;
		return {};
	}
	else
	{//TODO leak check
		YRet yr;
		YObj* yo = new YObj;
		yr.single.tp = YEArg::Object;
		yo->name.SetStr(GetModuleDesc().name);
		yo->obj = fo;
		yr.single.o = yo;
		return yr;
	}
}

YRet Close(YArgs* args)
{//TODO leak check
	Variable* me = (Variable*)args->args[0].o;
	FileObj* fo = (FileObj*)me->modObj()._o;
	if(!fo)
		return { 1, };

	int ec = fclose(fo->fp) ? errno : 0;
	if(!ec)
	{
		delete fo;
		me->modObj()._o = nullptr;
	}
	return { ec };
}

YRet IsBinaryMode(YArgs* args)
{
	Variable* me = (Variable*)args->args[0].o;
	FileObj* fo = (FileObj*)me->modObj()._o;
	if(!fo) return { 1, };

	auto rv = (Variable*)args->retBuff.o;
	rv->SetBool(fo->binary);
	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

YRet Read(YArgs* args)
{
	Variable* me = (Variable*)args->args[0].o;
	Variable* sz = (Variable*)args->args[1].o;

	FileObj* fo = (FileObj*)me->modObj()._o;
	if(!fo)
		return { 1, };

	auto rv = (Variable*)args->retBuff.o;
	if(fo->binary)
	{
		Variable v;
		v.SetBytes(sz->int_());
		size_t rsz = fread(v.bytes().data(), 1, sz->int_(), fo->fp);
		v.bytes().resize(rsz);
		rv->SetVar(v);
	}
	else
	{
		std::string s;
		s.resize(sz->int_());
		size_t rsz = fread(s.data(), 1, sz->int_(), fo->fp);
		s.resize(rsz);
		rv->SetStr(s);
	}

	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

YRet Write(YArgs* args)
{
	Variable* me = (Variable*)args->args[0].o;
	MODARG_VAR2(1, bf, Variable::STR, Variable::BYTES);

	FileObj* fo = (FileObj*)me->modObj()._o;
	if(!fo)
		return { 1, };

	size_t wsz = 0;
	int code = 0;
	if(bf == Variable::STR)
	{
		if(bf != Variable::STR)
			throw RuntimeError::IncorrectParam(bf._type, Variable::STR, 1);

		wsz = fwrite(bf.str().c_str(), 1, bf.str().size(), fo->fp);
		if(wsz < bf.str().size()) code = errno ? errno : 1;
	}
	else
	{
		if(bf != Variable::BYTES)
			throw RuntimeError::IncorrectParam(bf._type, Variable::BYTES, 1);

		wsz = fwrite(bf.bytes().data(), 1, bf.bytes().size(), fo->fp);
		if(wsz < bf.bytes().size()) code = errno ? errno : 1;
	}
	return { code };
}

YRet ReadAll(YArgs* args)
{
	MODARG_VAR(0, path, Variable::STR);
	bool isBin = false;
	if(args->numArgs > 1)
	{
		MODARG_VAR2(1, bin, Variable::_TRUE_, Variable::_FALSE_);
		isBin = bin == Variable::_TRUE_;
	}

	error_code ec;
	uintmax_t sz = filesystem::file_size(path.str(), ec);
	if(ec)
		return { ec.value(), };

	auto rv = (Variable*)args->retBuff.o;
	if(!isBin)
	{
		FILE* fp = fopen(path.str().c_str(), "r");
		if(fp == nullptr)
			return { errno, };


		rv->SetStr("");
		rv->str().resize(sz);
		uintmax_t rsz = fread(rv->str().data(), 1, sz, fp);
		bool eof = feof(fp);
		fclose(fp);

		if(rsz != sz)
		{//TODO
			if(eof)
			{
				rv->str().resize(rsz);
			}
			else
			{
				return { errno ? errno : 1, };
			}
		}
	}
	else
	{
		FILE* fp = fopen(path.str().c_str(), "rb");
		if(fp == nullptr)
			return { errno, };


		rv->SetBytes(sz);
		uintmax_t rsz = fread(rv->bytes().data(), 1, sz, fp);
		fclose(fp);

		if(rsz != sz)
		{//TODO
			return { errno ? errno : 1, };
		}
	}

	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

YRet WriteAll(YArgs* args)
{
	MODARG_VAR(0, path, Variable::STR);
	MODARG_VAR2(1, data, Variable::STR, Variable::BYTES);

	FILE* fp = fopen(path.str().c_str(), data == Variable::STR ? "w" : "wb");
	if(fp == nullptr)
		return { errno, };

	int code = 0;
	size_t wsz = 0;
	if(data == Variable::STR)
	{
		wsz = fwrite(data.str().c_str(), 1, data.str().size(), fp);
		if(wsz < data.str().size()) code = errno ? errno : 1;
	}
	else
	{
		wsz = fwrite(data.bytes().data(), 1, data.bytes().size(), fp);
		if(wsz < data.bytes().size()) code = errno ? errno : 1;
	}
	fclose(fp);
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
		m.memberTbl["is_binary_mode"] = ModuleMemberDesc{ "is_binary_mode", ModuleMemberDesc::FUNC, true, 0, IsBinaryMode };
		m.memberTbl["read"] = ModuleMemberDesc{ "read", ModuleMemberDesc::FUNC, true, 1, Read };
		m.memberTbl["write"] = ModuleMemberDesc{ "write", ModuleMemberDesc::FUNC, true, 1, Write };
		m.memberTbl["read_all"] = ModuleMemberDesc{ "read_all", ModuleMemberDesc::FUNC, false, 1, ReadAll };
		m.memberTbl["write_all"] = ModuleMemberDesc{ "write_all", ModuleMemberDesc::FUNC, false, 2, WriteAll };
	}
	return m;
}

}
