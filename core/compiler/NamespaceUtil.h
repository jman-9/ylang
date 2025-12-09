#pragma once
#include <string>


namespace ycom::NamespaceUtil
{
using namespace std;

struct Resolution
{
	string absPath;
	string namespacePath;
};

Resolution ResolveInclude(const string& incStr);

}
