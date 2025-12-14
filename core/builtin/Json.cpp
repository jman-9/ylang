#include "Json.h"
#include "vm/Variable.h"
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
	struct VNode
	{
		Variable* parentList = nullptr;
		Variable* node = nullptr;
		int pos = 0;

		Variable* get() { return node ? node : &(parentList->list()[pos]); }
	};

	auto AppendNode = [](Variable& v, const string* k = nullptr) -> VNode {
		if(v == Variable::LIST)
		{
			v.list().push_back({});
			return { .parentList = &v, .pos = (int)v.list().size()-1 };
		}
		else if(v == Variable::DICT)
		{
			auto inserted = v.dict().insert({ *k, {}});
			return { .node = &inserted.first->second };
		}
		else
		{//TODO exception system
			throw 'n';
		}
	};

	Variable* jsonStr = (Variable*)args->args[0].o;
	auto j = json::parse(jsonStr->str());

	Variable* vjson = (Variable*)args->retBuff.o;
	Variable* vtrav;
	queue<VNode> vq;

	queue<json*> jq;
	json* jtrav;

	jq.push(&j);
	vq.push({.node = vjson });
	for( ; !jq.empty(); )
	{
		jtrav = jq.front();
		VNode vn = vq.front();
		if(vn.parentList)
			vtrav = &vn.parentList->list()[vn.pos];
		else
			vtrav = vn.node;
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
				vn = AppendNode(*vtrav, &k);

			#ifdef DEBUG_OUT
				cout << " ";
			#endif

				jq.push(&v);
				vq.push(vn);
			}
			else
			{
				Variable* newV = AppendNode(*vtrav, &k).get();

				if(v.is_number_integer())
					newV->SetInt(v);
				else if(v.is_number_float())
					newV->SetFloat(v);
				else if(v.is_boolean())
				{//TODO ugly...
					newV->Clear();
					newV->_type = v == true ? Variable::_TRUE_ : Variable::_FALSE_;
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
	Variable* vobj = (Variable*)args->args[0].o;

	int indent = -1;
	if(args->numArgs > 1)
		indent = ((Variable*)args->args[1].o)->int_();

	Variable* vtrav;
	queue<Variable*> vq;

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

		if(*vtrav == Variable::LIST)
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
		else if(*vtrav == Variable::DICT)
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
			if(*vtrav == Variable::INT)
				*jtrav = vtrav->int_();
			else if(*vtrav == Variable::FLOAT)
				*jtrav = vtrav->float_();
			else if(*vtrav == Variable::_TRUE_ || *vtrav == Variable::_FALSE_)
				*jtrav = *vtrav == Variable::_TRUE_ ? true : false;
			else
				*jtrav = vtrav->ToStr();

		#ifdef DEBUG_OUT
			cout << " " << vtrav->ToStr() << endl;
		#endif
		}
	}

	YRet yr;
	yr.single.tp = YEArg::YVar;
	Variable* vdump =  (Variable*)args->retBuff.o;
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
