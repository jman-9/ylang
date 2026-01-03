#include "Path.h"
#include "vm/Variable.h"
#include "vm/RuntimeError.h"
#include "module/ModuleUtil.h"
#include <filesystem>


namespace ybuiltin::Fs::Path
{
using namespace yvm;
using namespace std;


YRet Join(YArgs* args)
{
	auto rv = (Variable*)args->retBuff.o;
	rv->SetStr("");

	if(args->numArgs > 0)
	{
		MODARG_VAR(0, path, Variable::STR);
		filesystem::path fullPath = path.str();
		for(int i=1; i<args->numArgs; i++)
		{
			MODARG_VAR(i, path, Variable::STR);
			fullPath /= path.str();
		}
		rv->_u._s->assign(fullPath.string());
	}

	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

YRet Parent(YArgs* args)
{
	MODARG_VAR(0, path, Variable::STR);

	auto rv = (Variable*)args->retBuff.o;
	rv->SetStr(filesystem::path(path.str()).parent_path().string());
	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}
YRet Name(YArgs* args)
{
	MODARG_VAR(0, path, Variable::STR);

	auto rv = (Variable*)args->retBuff.o;
	rv->SetStr(filesystem::path(path.str()).filename().string());
	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}
YRet Stem(YArgs* args)
{
	MODARG_VAR(0, path, Variable::STR);

	auto rv = (Variable*)args->retBuff.o;
	rv->SetStr(filesystem::path(path.str()).stem().string());
	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}
YRet Ext(YArgs* args)
{
	MODARG_VAR(0, path, Variable::STR);

	auto rv = (Variable*)args->retBuff.o;
	rv->SetStr(filesystem::path(path.str()).extension().string());
	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

const ymod::ModuleDesc& GetModuleDesc()
{
	static ymod::ModuleDesc m;

	if(m.name.empty())
	{
		m.name = "path";
		m.builtin = true;
		m.memberTbl[ "join" ] = { "join", ymod::ModuleMemberDesc::FUNC, false, 0, Join };
		m.memberTbl[ "parent" ] = { "parent", ymod::ModuleMemberDesc::FUNC, false, 1, Parent };
		m.memberTbl[ "name" ] = { "name", ymod::ModuleMemberDesc::FUNC, false, 1, Name };
		m.memberTbl[ "stem" ] = { "stem", ymod::ModuleMemberDesc::FUNC, false, 1, Stem };
		m.memberTbl[ "ext" ] = { "ext", ymod::ModuleMemberDesc::FUNC, false, 1, Ext };
	}
	return m;
}

}
