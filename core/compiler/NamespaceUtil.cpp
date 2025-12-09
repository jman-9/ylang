#include "NamespaceUtil.h"
#include "util/StrUtil.h"
#include <string>
#include <filesystem>


namespace ycom::NamespaceUtil
{
using namespace std;


const string& Context::Get() const { return _namespacePath; }
bool Context::IsEmpty() const { return _namespacePath.empty(); }
void Context::Clear() { _namespacePath.clear(); }
void Context::Append(const std::string& ns) { _namespacePath = _namespacePath.empty() ? ns : (_namespacePath + "." + ns); }

bool Tracker::IsTerminal(const Context& ctx) const
{
	auto found = _namespaceMap.find(ctx.Get());
	return found == _namespaceMap.end() ? true : found->second;
}
bool Tracker::IsExisting(const Context& ctx) const
{
	return _namespaceMap.contains(ctx.Get());
}
bool Tracker::IsExistingIfAppend(const Context& ctx, const string& toAppend) const
{
	return _namespaceMap.contains(ctx.IsEmpty() ? toAppend : ctx.Get() + "." + toAppend);
}
void Tracker::AddTrackingPath(const string& path)
{
	auto split = StrUtil::Split(path, ".");
	if(split.empty()) return;

	string nsStr = split[0];
	_namespaceMap[ nsStr ] = false;
	for(size_t i=1; i<split.size(); i++)
	{
		nsStr += "." + split[i];
		_namespaceMap[ nsStr ] = false;
	}
	_namespaceMap[ nsStr ] = true;
}


Resolution ResolveInclude(const string& incStr)
{
	Resolution res;

	auto pos = incStr.find_last_of('/');
	if(pos == string::npos)
	{
		res.absPath = StrUtil::Replace(incStr, ".", "/");
		res.absPath = filesystem::absolute(res.absPath).string();
		res.namespacePath = incStr;
	}
	else
	{
		string sub = incStr.substr(pos+1);
		res.absPath = incStr.substr(0, pos+1) + StrUtil::Replace(incStr.substr(pos+1), ".", "/");
		res.absPath = filesystem::absolute(res.absPath).string();
		res.namespacePath = sub;
	}
	return res;
}

}
