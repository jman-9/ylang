#include "Json.h"
#include "vm/Variable.h"
#include "ext/nlohmann/json.hpp"
using json = nlohmann::json;


namespace ybuiltin::Json
{
using namespace yvm;
using namespace ymod;

YRet Parse(YArgs* args)
{
	Variable* jsonStr = (Variable*)args->args[0].o;
	auto j = json::parse(jsonStr->_str);
	//TODO BFS
	return {};
}

YRet Dump(YArgs* args)
{//TODO BFS
	//Variable* json = (Variable*)args->args[0].o;
	return {};
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
