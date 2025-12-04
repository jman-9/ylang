#pragma once
#include "Str.h"
#include "vm/Variable.h"
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
	rv->SetInt((int64_t)self->str().empty());
	yr.single.tp = YEArg::YVar;
	yr.single.o = rv;
	return yr;
}

inline YRet Len(YArgs* args)
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
		throw 'n';//TODO

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
			throw 'n';
		}
		if(*i != Variable::INT)
		{
			throw 'n';
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
		throw 'n';//TODO

	YRet yr;
	yr.single.tp = YEArg::YVar;
	if(args->numArgs == 2)
	{
		auto s = (Variable*)args->args[1].o;
		if(*s != Variable::INT)
		{
			throw 'n';
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
			throw 'n';
		}
		if(*l != Variable::INT)
		{
			throw 'n';
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
		throw 'n';//TODO

	auto o = (Variable*)args->args[1].o;
	auto n =(Variable*)args->args[2].o;
	if(*o != Variable::STR)
	{
		throw 'n';
	}
	if(*n != Variable::STR)
	{
		throw 'n';
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
			throw 'n';
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
		throw 'n';//TODO

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


const ymod::ModuleDesc& GetModuleDesc()
{
	static ModuleDesc m;
	if(m.name.empty())
	{
		m.name = "str";
		m.builtin = true;
		m.memberTbl[ "empty" ] = { "empty", ymod::ModuleMemberDesc::FUNC, true, 0, Empty };
		m.memberTbl[ "len" ] = { "len", ymod::ModuleMemberDesc::FUNC, true, 0, Len };
		m.memberTbl[ "find" ] = { "find", ymod::ModuleMemberDesc::FUNC, true, 1, Find };
		m.memberTbl[ "substr" ] = { "substr", ymod::ModuleMemberDesc::FUNC, true, 1, Substr };
		m.memberTbl[ "replace" ] = { "replace", ymod::ModuleMemberDesc::FUNC, true, 2, Replace };
		m.memberTbl[ "split" ] = { "split", ymod::ModuleMemberDesc::FUNC, true, 0, Split };
		m.memberTbl[ "trim" ] = { "trim", ymod::ModuleMemberDesc::FUNC, true, 0, Trim };
		m.memberTbl[ "ltrim" ] = { "ltrim", ymod::ModuleMemberDesc::FUNC, true, 0, LTrim };
		m.memberTbl[ "rtrim" ] = { "rtrim", ymod::ModuleMemberDesc::FUNC, true, 0, RTrim };
		m.memberTbl[ "join" ] = { "join", ymod::ModuleMemberDesc::FUNC, true, 1, Join };
	}
	return m;
}

}
