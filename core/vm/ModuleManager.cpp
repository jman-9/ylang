#include "ModuleManager.h"
#include <format>
using namespace std;


#if defined(_WIN32)
#define NOGDI
#define NOMINMAX
#define NOCRYPT
#define NODRAWTEXT
#define NOHELP
#define NOPROXYSTUB
#define _NO_CRT_STDIO_INLINE
#define WIN32_LEAN_AND_MEAN
#define RPC_NO_WINDOWS_H
#include <windows.h>
#define LOAD_LIB(name) LoadLibraryA(name)
#define LOAD_SYM(lib, sym) GetProcAddress((HMODULE)lib, sym)
#define CLOSE_LIB(lib) FreeLibrary((HMODULE)lib)
#else
#include <dlfcn.h>
#define LOAD_LIB(name) dlopen(name, RTLD_NOW)
#define LOAD_SYM(lib, sym) dlsym(lib, sym)
#define CLOSE_LIB(lib) dlclose(lib)
#endif

using YModFn = YResult (*)(YObject*);




namespace yvm
{

ModuleManager::ModuleManager()
{

}

ModuleManager::~ModuleManager()
{

}

bool ModuleManager::Load(const string& mod)
{//todo cwd
	string modPath = format("y.{}.dll", mod);

	void* handle = LOAD_LIB(modPath.c_str());
	if (!handle)
	{
		throw 'n';
	}

	YModFn v = (YModFn)LOAD_SYM(handle, "version");
	YModFn p = (YModFn)LOAD_SYM(handle, "platform");

	auto r = v(nullptr);

	int a = 1;
	return true;
}

}
