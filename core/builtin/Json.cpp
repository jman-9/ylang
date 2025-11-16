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
	Variable* jsonStr = (Variable*)args->args[0].o;
	auto j = json::parse(jsonStr->_str);


	Variable* vjson = new Variable;
	Variable* vtrav;
	queue<Variable*> vq;

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
				Variable* node = new Variable;
				if(*vtrav == Variable::LIST)
				{
					vtrav->_list->push_back(node);
				}
				else if(*vtrav == Variable::DICT)
				{
					(*vtrav->_dict)[ k ] = node;
				}
				else
				{//TODO
					throw 'n';
				}

			#ifdef DEBUG_OUT
				cout << " ";
			#endif

				jq.push(&v);
				vq.push(node);
			}
			else
			{
				Variable* newV = new Variable;
				if(v.is_number_integer())
					newV->SetInt(v);
				else if(v.is_number_float())
					newV->SetFloat(v);
				else if(v.is_boolean())
					newV->SetInt(v);
				else
					newV->SetStr(v);

				if(*vtrav == Variable::LIST)
				{
					vtrav->_list->push_back(newV);
				}
				else if(*vtrav == Variable::DICT)
				{
					(*vtrav->_dict)[ k ] = newV;
				}
				else
				{//TODO
					throw 'n';
				}

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

	Variable* vindent = nullptr;
	if(args->numArgs > 1)
		vindent = (Variable*)args->args[1].o;

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

			for(auto& v : *vtrav->_list)
			{
			#ifdef DEBUG_OUT
				cout << i++ << " ";
			#endif

				jtrav->push_back("");
				vq.push(v);
			}
			for(auto& j : *jtrav)
			{
				jq.push(&j);
			}
		}
		else if(*vtrav == Variable::DICT)
		{
			for(auto& [k, v] : *vtrav->_dict)
			{
			#ifdef DEBUG_OUT
				cout << k << " ";
			#endif

				(*jtrav)[k] = json();
				jq.push(&(*jtrav)[k]);
				vq.push(v);
			}
		}
		else
		{
			if(*vtrav == Variable::INT)
				*jtrav = vtrav->_int;
			else if(*vtrav == Variable::FLOAT)
				*jtrav = vtrav->_float;
			//else if(v.is_boolean())//TODO
				//newV->SetInt(v);
			else
				*jtrav = vtrav->ToStr();

		#ifdef DEBUG_OUT
			cout << " " << vtrav->ToStr() << endl;
		#endif
		}
	}

	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = Variable::NewStr(jjson.dump(vindent ? vindent->_int : -1));
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
