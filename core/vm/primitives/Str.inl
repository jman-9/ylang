#pragma once
#include "Str.h"
#include "vm/Variable.h"
#include "vm/RuntimeError.h"
#include "module/ModuleUtil.h"
#include "util/StrUtil.h"
#include <string>


namespace yvm::primitive::Str
{
using namespace yvm;
using namespace ymod;
using namespace std;

inline YRet Empty(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	YRet yr;
	auto rv = (Variable*)args->retBuff.o;
	rv->SetBool(self->str().empty());
	yr.single.tp = YEArg::YVar;
	yr.single.o = rv;
	return yr;
}

inline YRet Size(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	YRet yr;
	auto rv = (Variable*)args->retBuff.o;
	rv->SetInt((int64_t)self->str().size());
	yr.single.tp = YEArg::YVar;
	yr.single.o = rv;
	return yr;
}

inline YRet Find(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	if(args->numArgs == 1)
		INTERNALERR_NUMARGS(2, args->numArgs);

	YRet yr;
	yr.single.tp = YEArg::YVar;
	if(args->numArgs == 2)
	{
		auto s = (Variable*)args->args[1].o;

		size_t pos = self->str().find(s->str());

		auto rv = (Variable*)args->retBuff.o;
		rv->SetInt(pos == string::npos ? -1 : pos);
		yr.single.o = rv;
	}
	else
	{
		auto i = (Variable*)args->args[1].o;
		auto s =(Variable*)args->args[2].o;
		if(*s != Variable::STR)
		{
			INTERNALERR_TYPE(s->TypeStr(), Variable::TypeStr(Variable::STR));
		}
		if(*i != Variable::INT)
		{
			INTERNALERR_TYPE(i->TypeStr(), Variable::TypeStr(Variable::INT));
		}

		size_t pos = self->str().find(s->str(), i->int_());

		auto rv = (Variable*)args->retBuff.o;
		rv->SetInt(pos == string::npos ? -1 : pos);
		yr.single.o = rv;
	}

	return yr;
}

inline YRet Substr(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	if(args->numArgs == 1)
		INTERNALERR_NUMARGS(2, args->numArgs);

	YRet yr;
	yr.single.tp = YEArg::YVar;
	if(args->numArgs == 2)
	{
		auto s = (Variable*)args->args[1].o;
		if(*s != Variable::INT)
		{
			INTERNALERR_TYPE(s->TypeStr(), Variable::TypeStr(Variable::INT));
		}

		auto rv = (Variable*)args->retBuff.o;
		rv->SetStr(self->str().substr(s->int_()));
		yr.single.o = rv;
	}
	else
	{
		auto s = (Variable*)args->args[1].o;
		auto l =(Variable*)args->args[2].o;
		if(*s != Variable::INT)
		{
			INTERNALERR_TYPE(s->TypeStr(), Variable::TypeStr(Variable::INT));
		}
		if(*l != Variable::INT)
		{
			INTERNALERR_TYPE(l->TypeStr(), Variable::TypeStr(Variable::INT));
		}

		auto rv = (Variable*)args->retBuff.o;
		rv->SetStr(self->str().substr(s->int_(), l->int_()));
		yr.single.o = rv;
	}

	return yr;
}

inline YRet Replace(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	if(args->numArgs < 3)
		INTERNALERR_NUMARGS(3, args->numArgs);

	auto o = (Variable*)args->args[1].o;
	auto n =(Variable*)args->args[2].o;
	if(*o != Variable::STR)
	{
		INTERNALERR_TYPE(o->TypeStr(), Variable::TypeStr(Variable::STR));
	}
	if(*n != Variable::STR)
	{
		INTERNALERR_TYPE(n->TypeStr(), Variable::TypeStr(Variable::STR));
	}

	string r = self->str();
	if(!o->str().empty())
	{
		size_t pos = 0;
		for(size_t pos=0; (pos = r.find(o->str(), pos)) != std::string::npos; ) {
			r.replace(pos, o->str().length(), n->str());
			pos += n->str().length();
		}
	}

	YRet yr;
	yr.single.tp = YEArg::YVar;
	auto rv = (Variable*)args->retBuff.o;
	rv->SetStr(r);
	yr.single.o = rv;
	return yr;
}

inline YRet Split(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	YRet yr;
	Variable ret;
	if(args->numArgs == 1)
	{
		const string& src = self->str();
		size_t start = src.find_first_not_of(" \t\n\r");
		size_t end = 0;

		ret.SetList();
		for( ; start != string::npos; )
		{
			end = src.find_first_of(" \t\n\r", start);
			ret.list().push_back({});
			ret.list().back().SetStr(src.substr(start, end - start));
			start = src.find_first_not_of(" \t\n\r", end);
		}
	}
	else
	{
		auto d = (Variable*)args->args[1].o;
		if(*d != Variable::STR)
		{
			INTERNALERR_TYPE(d->TypeStr(), Variable::TypeStr(Variable::STR));
		}

		if(d->str().empty())
		{
			ret.SetStr(self->str());
		}
		else
		{
			const string& src = self->str();
			size_t start = 0;
			size_t end = 0;

			ret.SetList();
			for( ; (end = src.find(d->str(), start)) != std::string::npos; )
			{
				ret.list().push_back({});
				ret.list().back().SetStr(src.substr(start, end - start));
				start = end + d->str().length();
			}
			ret.list().push_back({});
			ret.list().back().SetStr(src.substr(start, end - start));
		}
	}

	yr.single.tp = YEArg::YVar;
	auto rv = (Variable*)args->retBuff.o;
	rv->SetVar(ret);
	yr.single.o = rv;
	return yr;
}


constexpr string_view kAsciiSpaces = " \t\n\r\f\v";

inline YRet Trim(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	const string& r = self->str();
	size_t start = r.find_first_not_of(kAsciiSpaces);
	size_t end = start == string_view::npos ? string_view::npos : r.find_last_not_of(kAsciiSpaces);

	YRet yr;
	yr.single.tp = YEArg::YVar;
	auto rv = (Variable*)args->retBuff.o;
	rv->SetStr(start == string_view::npos ? "" : r.substr(start, end - start + 1));
	yr.single.o = rv;
	return yr;
}
inline YRet LTrim(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	const string& r = self->str();
	size_t start = r.find_first_not_of(kAsciiSpaces);

	YRet yr;
	yr.single.tp = YEArg::YVar;
	auto rv = (Variable*)args->retBuff.o;
	rv->SetStr(start == string_view::npos ? "" : r.substr(start));
	yr.single.o = rv;
	return yr;
}
inline YRet RTrim(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	const string& r = self->str();
	size_t end = r.find_last_not_of(kAsciiSpaces);

	YRet yr;
	yr.single.tp = YEArg::YVar;
	auto rv = (Variable*)args->retBuff.o;
	rv->SetStr(end == string_view::npos ? "" : r.substr(0, end + 1));
	yr.single.o = rv;
	return yr;
}

inline YRet Join(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	if(args->numArgs < 2)
		INTERNALERR_NUMARGS(2, args->numArgs);

	auto list = (Variable*)args->args[1].o;

	string j;
	if(!list->list().empty())
	{
		j = list->list().at(0).ToStr();
		for(int i=1; i<list->list().size(); i++)
		{
			j += self->str() + list->list().at(i).ToStr();
		}
	}

	YRet yr;
	yr.single.tp = YEArg::YVar;

	auto vj = (Variable*)args->retBuff.o;
	vj->SetStr(j);
	yr.single.o = vj;
	return yr;
}

inline YRet StartsWith(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	MODARG_VAR(1, str, Variable::STR);

	const string& r = self->str();
	auto rv = (Variable*)args->retBuff.o;
	rv->SetBool(r.starts_with(str.str()));

	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}
inline YRet EndsWith(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	MODARG_VAR(1, str, Variable::STR);

	const string& r = self->str();
	auto rv = (Variable*)args->retBuff.o;
	rv->SetBool(r.ends_with(str.str()));

	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

inline YRet ToUpper(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	auto rv = (Variable*)args->retBuff.o;
	rv->SetStr(StrUtil::ToUpper(self->str()));

	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}
inline YRet ToLower(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	auto rv = (Variable*)args->retBuff.o;
	rv->SetStr(StrUtil::ToLower(self->str()));

	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

inline YRet ToInt(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	int base = 10;
	if(args->numArgs >= 2)
	{
		MODARG_VAR(1, vbase, Variable::INT);
		base = vbase.int_();
	}

	int64_t converted;
	try
	{
		converted = stoll(self->str(), nullptr, base);
	}
	catch(const std::exception& ee)
	{
		INTERNALERR(ee.what());
	}

	auto rv = (Variable*)args->retBuff.o;
	rv->SetInt(converted);

	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}

const ymod::ModuleDesc& GetModuleDesc()
{
	static ModuleDesc m;
	if(m.name.empty())
	{
		m.name = "str";
		m.builtin = true;
		m.memberTbl[ "empty" ] = { "empty", ymod::ModuleMemberDesc::FUNC, true, 0, Empty };
		m.memberTbl[ "size" ] = { "size", ymod::ModuleMemberDesc::FUNC, true, 0, Size };
		m.memberTbl[ "find" ] = { "find", ymod::ModuleMemberDesc::FUNC, true, 1, Find };
		m.memberTbl[ "substr" ] = { "substr", ymod::ModuleMemberDesc::FUNC, true, 1, Substr };
		m.memberTbl[ "replace" ] = { "replace", ymod::ModuleMemberDesc::FUNC, true, 2, Replace };
		m.memberTbl[ "split" ] = { "split", ymod::ModuleMemberDesc::FUNC, true, 0, Split };
		m.memberTbl[ "trim" ] = { "trim", ymod::ModuleMemberDesc::FUNC, true, 0, Trim };
		m.memberTbl[ "ltrim" ] = { "ltrim", ymod::ModuleMemberDesc::FUNC, true, 0, LTrim };
		m.memberTbl[ "rtrim" ] = { "rtrim", ymod::ModuleMemberDesc::FUNC, true, 0, RTrim };
		m.memberTbl[ "join" ] = { "join", ymod::ModuleMemberDesc::FUNC, true, 1, Join };
		m.memberTbl[ "starts_with" ] = { "starts_with", ymod::ModuleMemberDesc::FUNC, true, 1, StartsWith };
		m.memberTbl[ "ends_with" ] = { "ends_with", ymod::ModuleMemberDesc::FUNC, true, 1, EndsWith };
		m.memberTbl[ "toupper" ] = { "toupper", ymod::ModuleMemberDesc::FUNC, true, 0, ToUpper };
		m.memberTbl[ "tolower" ] = { "tolower", ymod::ModuleMemberDesc::FUNC, true, 0, ToLower };
		m.memberTbl[ "to_int" ] = { "to_int", ymod::ModuleMemberDesc::FUNC, true, 0, ToInt };
	}
	return m;
}

}
