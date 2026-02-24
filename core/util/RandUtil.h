#pragma once
#include <stdlib.h>
#include <string>

#ifdef _WIN32
#define RANDOM(__min__, __max__) ((int)(((double)((rand()<<15) | (rand()&0) | rand())) / ((RAND_MAX<<15 | RAND_MAX) + 1) * (((__max__) + 1) - (__min__))) + (__min__))

#define RANDOMD(__min__, __max__) ((((double)((rand()<<15) | (rand()&0) | rand())) / ((RAND_MAX<<15 | RAND_MAX) + 1) * ((((double)__max__)) - (double)(__min__))) + (double)(__min__))
#else
#define RANDOM(__min__, __max__) ((int64_t)rand() * ((__max__) + 1 - (__min__)) / ((int64_t)RAND_MAX + 1) + (__min__))
#define RANDOMD(__min__, __max__) ((double)rand() / ((int64_t)RAND_MAX + 1) *  (((double)(__max__)) - ((double)(__min__)))+ (double)(__min__))
#endif

#define RANDOMIZE_TIMER() srand( (unsigned int)time( NULL ) )


namespace RandUtil
{
using namespace std;

inline string GetRandBase64Str(int len);
}

#include "RandUtil.inl"
