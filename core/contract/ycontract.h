#pragma once

#if defined(_WIN32)
#define EXPORT __declspec(dllexport)
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
#define EXPORT __attribute__((visibility("default")))
#include <dlfcn.h>
#define LOAD_LIB(name) dlopen(name, RTLD_NOW)
#define LOAD_SYM(lib, sym) dlsym(lib, sym)
#define CLOSE_LIB(lib) dlclose(lib)
#endif

#include <stdint.h>
#include <string>
#include <vector>


enum class YEArg : uint32_t
{
	None,

	YVar,	// for zero-copy

	Self,
	Int64,
	Double,
	Str,
	List,
	Dict,
	Object,
};


struct YArg
{
	void* o = nullptr;
	YEArg tp = YEArg::None;

	int64_t ToInt64() const;
	double ToDouble() const;
	std::string ToStr() const;
	std::vector<YArg> ToList() const;

	bool SetInt64(int64_t n);
	bool SetDouble(double d);
	bool SetStr(const std::string& s);
	bool SetYVar(void* yv);
	//bool FromList() const;
};

struct YStr
{
	int len = 0;
	char* str = nullptr;

	void SetStr(const std::string& s);
};

struct YList
{
	int sz = 0;
	YArg* list = nullptr;
};

struct YDict
{
	int sz = 0;
	YArg* keys = nullptr;
	YArg* vals = nullptr;
};

struct YArgs
{
	int numArgs = 0;
	YArg* args = nullptr;

	void Reset(int num);
};

struct YRet
{
	int code = 0;
	YList vals;
	YArg single;
};

struct YObj
{
	YStr name;
	void* obj;
};

using YModFn = YRet (*)(YArgs*);

#include "ycontract.inl"
