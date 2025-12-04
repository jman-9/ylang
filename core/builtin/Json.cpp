#include "Json.h"
#include "vm/Variable2.h"
#include "ext/nlohmann/json.hpp"
#include <queue>
#include <iostream>


//#define DEBUG_OUT


namespace ybuiltin::Json
{
using json = nlohmann::json;
using namespace yvm;
using namespace ymod;
using namespace std;

YRet Parse(YArgs* args)
{
	auto AppendNode = [](Variable2& v, const string* k = nullptr) -> Variable2* {
		if(v == Variable2::LIST)
		{
			v.list().push_back({});
			return &v.list().back();
		}
		else if(v == Variable2::DICT)
		{
			auto inserted = v.dict().insert({ *k, {}});
			return &inserted.first->second;
		}
		else
		{//TODO
			throw 'n';
		}
	};

	Variable2* jsonStr = (Variable2*)args->args[0].o;
	auto j = json::parse(jsonStr->str());


	Variable2* vjson = (Variable2*)args->retBuff.o;
	Variable2* vtrav;
	queue<Variable2*> vq;

	queue<json*> jq;
	json* jtrav;

	jq.push(&j);
	vq.push(vjson);
	for( ; !jq.empty(); )
	{
		jtrav = jq.front();
		vtrav = vq.front();
		jq.pop();
		vq.pop();

		if(jtrav->is_array())
		{
			vtrav->SetList();
		}
		else if(jtrav->is_object())
		{
			vtrav->SetDict();
		}

		for(auto& [k, v] : jtrav->items())
		{
		#ifdef DEBUG_OUT
			cout << k;
		#endif

			if(v.is_array() || v.is_object())
			{
				Variable2* node = AppendNode(*vtrav, &k);

			#ifdef DEBUG_OUT
				cout << " ";
			#endif

				jq.push(&v);
				vq.push(node);
			}
			else
			{
				Variable2* newV = AppendNode(*vtrav, &k);
				if(v.is_number_integer())
					newV->SetInt(v);
				else if(v.is_number_float())
					newV->SetFloat(v);
				else if(v.is_boolean())
				{//TODO ugly...
					newV->Clear();
					newV->_type = v == true ? Variable2::_TRUE_ : Variable2::_FALSE_;
				}
				else
					newV->SetStr(v);

			#ifdef DEBUG_OUT
				cout << " " << v << endl;
			#endif
			}
		}
	}

	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = vjson;
	return yr;
}

YRet Dump(YArgs* args)
{
	Variable2* vobj = (Variable2*)args->args[0].o;

	int indent = -1;
	if(args->numArgs > 1)
		indent = ((Variable2*)args->args[1].o)->int_();

	Variable2* vtrav;
	queue<Variable2*> vq;

	json jjson;
	queue<json*> jq;
	json* jtrav;

	jq.push(&jjson);
	vq.push(vobj);
	for( ; !jq.empty(); )
	{
		jtrav = jq.front();
		vtrav = vq.front();
		jq.pop();
		vq.pop();

		if(*vtrav == Variable2::LIST)
		{
			int i = 0;

			for(auto& v : vtrav->list())
			{
			#ifdef DEBUG_OUT
				cout << i++ << " ";
			#endif

				jtrav->push_back(json());
				vq.push(&v);
			}
			for(auto& j : *jtrav)
			{
				jq.push(&j);
			}
		}
		else if(*vtrav == Variable2::DICT)
		{
			for(auto& [k, v] : vtrav->dict())
			{
			#ifdef DEBUG_OUT
				cout << k << " ";
			#endif

				(*jtrav)[k] = json();
				jq.push(&(*jtrav)[k]);
				vq.push(&v);
			}
		}
		else
		{
			if(*vtrav == Variable2::INT)
				*jtrav = vtrav->int_();
			else if(*vtrav == Variable2::FLOAT)
				*jtrav = vtrav->float_();
			else if(*vtrav == Variable2::_TRUE_ || *vtrav == Variable2::_FALSE_)
				*jtrav = *vtrav == Variable2::_TRUE_ ? true : false;
			else
				*jtrav = vtrav->ToStr();

		#ifdef DEBUG_OUT
			cout << " " << vtrav->ToStr() << endl;
		#endif
		}
	}

	YRet yr;
	yr.single.tp = YEArg::YVar;
	Variable2* vdump =  (Variable2*)args->retBuff.o;
	vdump->SetStr(jjson.dump(indent));
	yr.single.o = vdump;
	return yr;
}

const ModuleDesc& GetModuleDesc()
{
	static ModuleDesc m;

	if(m.name.empty())
	{
		m.name = "json";
		m.builtin = true;
		m.memberTbl["parse"] = { "parse", ModuleMemberDesc::FUNC, false, 1, Parse };
		m.memberTbl["dump"] = { "dump", ModuleMemberDesc::FUNC, false, 1, Dump };
	}
	return m;
}

}
