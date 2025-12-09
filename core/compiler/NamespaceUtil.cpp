#include "NamespaceUtil.h"
#include "util/StrUtil.h"
#include <string>
#include <filesystem>


namespace ycom::NamespaceUtil
{
using namespace std;

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
