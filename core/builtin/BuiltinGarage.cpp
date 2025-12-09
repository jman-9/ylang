#include "BuiltinGarage.h"
#include "Sys.h"
#include "Math.h"
#include "Rand.h"
#include "File.h"
#include "Json.h"
#include <unordered_set>
using namespace std;


static std::unordered_set<std::string> s_builtinMap;
static bool Init()
{
	using namespace ybuiltin;

	s_builtinMap.insert(Sys::GetModuleDesc().name);
	s_builtinMap.insert(Math::GetModuleDesc().name);
	s_builtinMap.insert(Rand::GetModuleDesc().name);
	s_builtinMap.insert(File::GetModuleDesc().name);
	s_builtinMap.insert(Json::GetModuleDesc().name);
	return true;
}
static bool init = Init();


bool ybuiltin::Garage::RegisterAll(ymod::ModuleManager& retModMgr)
{
	if(!retModMgr.RegisterModuleDesc(Sys::GetModuleDesc())) return false;
	if(!retModMgr.RegisterModuleDesc(Math::GetModuleDesc())) return false;
	if(!retModMgr.RegisterModuleDesc(Rand::GetModuleDesc())) return false;
	if(!retModMgr.RegisterModuleDesc(File::GetModuleDesc())) return false;
	if(!retModMgr.RegisterModuleDesc(Json::GetModuleDesc())) return false;
	return true;
}

bool ybuiltin::Garage::IsBuiltin(const std::string& name)
{
	return s_builtinMap.contains(name);
}
