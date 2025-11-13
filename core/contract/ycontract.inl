#pragma once
#include "ycontract.h"

inline int64_t YObj::ToInt64() const
{
	return (int64_t)o;
}

inline double YObj::ToDouble() const
{
	return (double)(intptr_t)o;
}

inline std::string YObj::ToStr() const
{
	return std::string(static_cast<YStr*>(o)->str, static_cast<YStr*>(o)->len);
}

inline std::vector<YObj> YObj::ToList() const
{
	YList* yl = (YList*)o;
	std::vector<YObj> vt(yl->sz);
	memcpy(vt.data(), yl->list, yl->sz * sizeof(YObj));
	return vt;
}

inline bool YObj::FromInt64(int64_t n)
{
	if(o) return false;//TODO
	tp = YEObj::Int64;
	o = (void*)(intptr_t)n;
	return true;
}

inline bool YObj::FromDouble(double d)
{
	if(o) return false;//TODO
	tp = YEObj::Double;
	o = (void*)(intptr_t)d;
	return true;
}

inline bool YObj::FromStr(const std::string& s)
{
	if(o) return false;//TODO
	tp = YEObj::Str;
	YStr* ys = new YStr;
	ys->FromStr(s);
	o = (void*)ys;
	return true;
}

inline void YStr::FromStr(const std::string& s)
{
	if(str)
		delete str;

	str = new char[s.size()+1];
	memcpy(str, s.c_str(), s.size()+1);
}

inline void YArgs::Reset(int num)
{
	if(args)
		delete[] args;

	numArgs = num;
	args = num > 0 ? new YObj[num] : nullptr;
}
