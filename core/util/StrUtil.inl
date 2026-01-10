#pragma once
#include "StrUtil.h"
#include <algorithm>


namespace StrUtil
{
using namespace std;

string Replace(const string& src, const string& old, const string& new_)
{
	string r = src;
	if(!old.empty())
	{
		size_t pos = 0;
		for(size_t pos=0; (pos = r.find(old, pos)) != string::npos; ) {
			r.replace(pos, old.length(), new_);
			pos += new_.length();
		}
	}
	return r;
}

vector<string> Split(const string& src, const string& delim /* = "" */, const string& delimChars /* = " \t\n\r" */)
{
	vector<string> v;

	if(delim.empty())
	{
		size_t start = src.find_first_not_of(delimChars);
		size_t end = 0;

		for( ; start != string::npos; )
		{
			end = src.find_first_of(delimChars, start);
			v.push_back(src.substr(start, end - start));
			start = src.find_first_not_of(delimChars, end);
		}
	}
	else
	{
		size_t start = 0;
		size_t end = 0;

		for( ; (end = src.find(delim, start)) != std::string::npos; )
		{
			v.push_back(src.substr(start, end - start));
			start = end + delim.length();
		}
		v.push_back(src.substr(start, end - start));
	}

	return v;
}


constexpr string_view kAsciiSpaces = " \t\n\r\f\v";

string Trim(const string& src)
{
	size_t start = src.find_first_not_of(kAsciiSpaces);
	size_t end = start == string_view::npos ? string_view::npos : src.find_last_not_of(kAsciiSpaces);

	return start == string_view::npos ? "" : src.substr(start, end - start + 1);
}
string LTrim(const string& src)
{
	size_t start = src.find_first_not_of(kAsciiSpaces);

	return start == string_view::npos ? "" : src.substr(start);
}
string RTrim(const string& src)
{
	size_t end = src.find_last_not_of(kAsciiSpaces);

	return end == string_view::npos ? "" : src.substr(0, end + 1);
}

string Join(const vector<string>& v, const string& delim)
{
	string j;
	if(!v.empty())
	{
		j = v[0];
		for(size_t i=1; i<v.size(); i++)
		{
			j += delim + v[i];
		}
	}
	return j;
}

string ToUpper(const string& src)
{
	string s = src;
	transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(toupper(c)); });
	return s;
}
string ToLower(const string& src)
{
	string s = src;
	transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(tolower(c)); });
	return s;
}

}
