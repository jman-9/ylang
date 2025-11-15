#pragma once
#include "Str.h"
#include "vm/Variable.h"
#include <string>


namespace yvm::primitive::Str
{
using namespace yvm;
using namespace ymod;
using namespace std;

inline YRet Len(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	YRet yr;
	auto rv = new Variable;
	rv->SetInt((int64_t)self->_str.size());
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

		size_t pos = self->_str.find(s->_str);

		yr.single.o = Variable::NewInt(pos == string::npos ? -1 : pos);
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

		size_t pos = self->_str.find(s->_str, i->_int);

		yr.single.o = Variable::NewInt(pos == string::npos ? -1 : pos);
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

		yr.single.o = Variable::NewStr(self->_str.substr(s->_int));
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

		yr.single.o = Variable::NewStr(self->_str.substr(s->_int, l->_int));
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

	string r = self->_str;
	if(!o->_str.empty())
	{
		size_t pos = 0;
		for(size_t pos=0; (pos = r.find(o->_str, pos)) != std::string::npos; ) {
			r.replace(pos, o->_str.length(), n->_str);
			pos += n->_str.length();
		}
	}

	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = Variable::NewStr(r);
	return yr;
}

inline YRet Split(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	YRet yr;
	Variable* ret;
	if(args->numArgs == 1)
	{
		const string& src = self->_str;
		size_t start = src.find_first_not_of(" \t\n\r");
		size_t end = 0;
		ret = Variable::NewList();

		for( ; start != string::npos; )
		{
			end = src.find_first_of(" \t\n\r", start);
			ret->_list->push_back(Variable::NewStr(src.substr(start, end - start)));
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

		if(d->_str.empty())
		{
			ret = Variable::NewStr(self->_str);
		}
		else
		{
			size_t start = 0;
			size_t end = 0;
			ret = Variable::NewList();

			const string& src = self->_str;
			for( ; (end = src.find(d->_str, start)) != std::string::npos; )
			{
				ret->_list->push_back(Variable::NewStr(src.substr(start, end - start)));
				start = end + d->_str.length();
			}
			ret->_list->push_back(Variable::NewStr(src.substr(start, end - start)));
		}
	}

	yr.single.tp = YEArg::YVar;
	yr.single.o = ret;
	return yr;
}


constexpr string_view kAsciiSpaces = " \t\n\r\f\v";

inline YRet Trim(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	const string& r = self->_str;
	size_t start = r.find_first_not_of(kAsciiSpaces);
	size_t end = start == string_view::npos ? string_view::npos : r.find_last_not_of(kAsciiSpaces);

	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = Variable::NewStr(start == string_view::npos ? "" : r.substr(start, end - start + 1));
	return yr;
}
inline YRet LTrim(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	const string& r = self->_str;
	size_t start = r.find_first_not_of(kAsciiSpaces);

	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = Variable::NewStr(start == string_view::npos ? "" : r.substr(start));
	return yr;
}
inline YRet RTrim(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;

	const string& r = self->_str;
	size_t end = r.find_last_not_of(kAsciiSpaces);

	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = Variable::NewStr(end == string_view::npos ? "" : r.substr(0, end + 1));
	return yr;
}

inline YRet Join(YArgs* args)
{
	auto self = (Variable*)args->args[0].o;
	if(args->numArgs < 2)
		throw 'n';//TODO

	auto list = (Variable*)args->args[1].o;

	Variable* vj = Variable::NewStr();
	string& j = vj->_str;
	if(!list->_list->empty())
	{
		j = list->_list->at(0)->ToStr();
		for(int i=1; i<list->_list->size(); i++)
		{
			j += self->_str + list->_list->at(i)->ToStr();
		}
	}

	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = vj;
	return yr;
}


const ymod::Module& GetModule()
{
	static Module m;
	if(m.name.empty())
	{
		m.name = "str";
		m.builtin = true;
		m.funcTbl[ "len" ] = { "len", true, 0, Len };
		m.funcTbl[ "find" ] = { "find", true, 1, Find };
		m.funcTbl[ "substr" ] = { "substr", true, 1, Substr };
		m.funcTbl[ "replace" ] = { "replace", true, 2, Replace };
		m.funcTbl[ "split" ] = { "split", true, 0, Split };
		m.funcTbl[ "trim" ] = { "trim", true, 0, Trim };
		m.funcTbl[ "ltrim" ] = { "ltrim", true, 0, LTrim };
		m.funcTbl[ "rtrim" ] = { "rtrim", true, 0, RTrim };
		m.funcTbl[ "join" ] = { "join", true, 1, Join };
	}
	return m;
}

}
