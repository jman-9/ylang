#pragma once
#include "ycontract.h"

inline int64_t YArg::ToInt64() const
{
	return (int64_t)o;
}

inline double YArg::ToDouble() const
{
	return *(double*)&o;
}

inline std::string YArg::ToStr() const
{
	return std::string(static_cast<YStr*>(o)->str, static_cast<YStr*>(o)->len);
}

inline std::vector<YArg> YArg::ToList() const
{
	YList* yl = (YList*)o;
	std::vector<YArg> vt(yl->sz);
	memcpy(vt.data(), yl->list, yl->sz * sizeof(YArg));
	return vt;
}

inline bool YArg::FromInt64(int64_t n)
{
	if(o) return false;//TODO
	tp = YEArg::Int64;
	o = (void*)(intptr_t)n;
	return true;
}

inline bool YArg::FromDouble(double d)
{
	if(o) return false;//TODO
	tp = YEArg::Double;
	o = (void*)*(intptr_t*)&d;
	return true;
}

inline bool YArg::FromStr(const std::string& s)
{
	if(o) return false;//TODO
	tp = YEArg::Str;
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
	args = num > 0 ? new YArg[num] : nullptr;
}
