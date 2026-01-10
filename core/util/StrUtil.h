#pragma once
#include <string>
#include <vector>

namespace StrUtil
{
	using namespace std;

	inline string Replace(const string& src, const string& old, const string& new_);
	inline vector<string> Split(const string& src, const string& delim = "", const string& delimChars = " \t\n\r");
	inline string Trim(const string& src);
	inline string LTrim(const string& src);
	inline string RTrim(const string& src);
	inline string Join(const vector<string>& v, const string& delim);
	inline string ToUpper(const string& src);
	inline string ToLower(const string& src);
}

#include "StrUtil.inl"
