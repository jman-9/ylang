#pragma once
#include <string>
#include <unordered_map>


namespace ycom::NamespaceUtil
{
using namespace std;


class Context
{
public:
	const string& Get() const;

	bool IsEmpty() const;
	void Clear();
	void Append(const std::string& ns);

protected:
	string _namespacePath;
};

class Tracker
{
public:
	bool IsTerminal(const Context& ctx) const;
	bool IsExisting(const Context& ctx) const;
	bool IsExistingIfAppend(const Context& ctx, const string& toAppend) const;

	void AddTrackingPath(const string& path);

protected:
	unordered_map<string, bool> _namespaceMap;
};


struct Resolution
{
	string absPath;
	string namespacePath;
};

Resolution ResolveInclude(const string& incStr, const string& baseDir);

}
