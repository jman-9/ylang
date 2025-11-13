#include "ycontract.h"


extern "C"
{
    EXPORT YRet version(YObj* obj);
    EXPORT YRet platform(YObj* obj);
}

EXPORT YRet version(YObj* obj)
{
    return {};
}

EXPORT YRet platform(YObj* obj)
{
    return {0, nullptr};
}
