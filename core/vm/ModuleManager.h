#pragma once
#include <string>


struct YObject {
	void* data;
};

struct YResult {
	int code;
	YObject* obj;
};


namespace yvm
{

class ModuleManager
{
public:
	ModuleManager();
	~ModuleManager();

	bool Load(const std::string& mod);
};

};
