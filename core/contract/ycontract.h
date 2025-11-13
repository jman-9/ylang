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


struct YObj
{
    void* data = nullptr;
};

struct YRet
{
    int code;
    YObj* obj = nullptr;
};
