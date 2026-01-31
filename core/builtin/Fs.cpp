#include "Fs.h"
#include "Path.h"
#include "vm/Variable.h"
#include "vm/RuntimeError.h"
#include "module/ModuleUtil.h"
#include <stdlib.h>
#include <filesystem>


namespace ybuiltin::Fs
{
using namespace yvm;
using namespace ymod;
using namespace std;


YRet Exists(YArgs* args)
{
	MODARG_VAR(0, path, Variable::STR);

	auto rv = (Variable*)args->retBuff.o;
	rv->SetBool(filesystem::exists(path.str()));
	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

YRet Cwd(YArgs* args)
{
	auto rv = (Variable*)args->retBuff.o;
	if(args->numArgs > 0)
	{
		MODARG_VAR(0, path, Variable::STR);
		if(!filesystem::exists(path.str()))
		{
			rv->SetBool(false);
		}
		else
		{
			filesystem::current_path(path.str());
			rv->SetBool(true);
		}
	}
	else
	{
		rv->SetStr(filesystem::current_path().string());
	}
	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

YRet AbsPath(YArgs* args)
{
	MODARG_VAR(0, path, Variable::STR);

	auto rv = (Variable*)args->retBuff.o;
	rv->SetStr(filesystem::absolute(path.str()).string());
	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

YRet ReadDir(YArgs* args)
{
	MODARG_VAR(0, path, Variable::STR);

	bool recursive = false;
	if(args->numArgs > 1)
	{
		const auto& v = *(Variable*)args->args[1].o;
		if(!v.IsBool())
		{//TODO
			throw RuntimeError::IncorrectParam(v._type, Variable::_TRUE_, 1);
		}
		recursive = v.bool_();
	}

	auto rv = (Variable*)args->retBuff.o;
	rv->SetList();
	if(recursive)
	{
		for(auto& d : filesystem::recursive_directory_iterator(path.str()))
		{
			if(!d.is_regular_file())
				continue;
			rv->list().push_back({});
			rv->list().back().SetStr(d.path().string());
		}
	}
	else
	{
		for(auto& d : filesystem::directory_iterator(path.str()))
		{
			if(!d.is_regular_file())
				continue;
			rv->list().push_back({});
			rv->list().back().SetStr(d.path().string());
		}
	}

	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

YRet MkDirs(YArgs* args)
{
	MODARG_VAR(0, path, Variable::STR);

	error_code ec;
	auto rv = (Variable*)args->retBuff.o;
	rv->SetBool(filesystem::create_directories(path.str(), ec));
	YRet yr;
	if(ec.value())
		yr.code = ec.value();
	yr.single.SetYVar(rv);
	return yr;
}

YRet Remove(YArgs* args)
{
	MODARG_VAR(0, path, Variable::STR);

	error_code ec;
	auto rv = (Variable*)args->retBuff.o;
	rv->SetBool(filesystem::remove(path.str(), ec));
	YRet yr;
	if(ec.value())
		yr.code = ec.value();
	yr.single.SetYVar(rv);
	return yr;
}


Module Init()
{//TODO memory leak
	Module o(&GetModuleDesc());
	Variable* v = new Variable;
	v->SetModule(Path::GetModuleDesc(), false);
	o.memberVars["path"] = YArg{v, YEArg::YVar};
	return o;
}

const ModuleDesc& GetModuleDesc()
{
	static ymod::ModuleDesc m;

	if(m.name.empty())
	{
		m.name = "fs";
		m.initer = Init;
		m.builtin = true;
		m.memberTbl[ "exists" ] = { "exists", ymod::ModuleMemberDesc::FUNC, false, 1, Exists };
		m.memberTbl[ "cwd" ] = { "cwd", ymod::ModuleMemberDesc::FUNC, false, 0, Cwd };
		m.memberTbl[ "abspath" ] = { "abspath", ymod::ModuleMemberDesc::FUNC, false, 1, AbsPath };
		m.memberTbl[ "readdir" ] = { "readdir", ymod::ModuleMemberDesc::FUNC, false, 1, ReadDir };
		m.memberTbl[ "mkdirs" ] = { "mkdirs", ymod::ModuleMemberDesc::FUNC, false, 1, MkDirs };
		m.memberTbl[ "remove" ] = { "remove", ymod::ModuleMemberDesc::FUNC, false, 1, Remove };
	}
	return m;
}

}
