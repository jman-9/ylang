#pragma once
#include "RandUtil.h"


namespace RandUtil
{
using namespace std;

const char base64url_chars[64] =
{
'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
'0','1','2','3','4','5','6','7','8','9','-','_'
};

string GetRandBase64Str(int len)
{
	string s = "";
	while(len--)
	{
		s += base64url_chars[RANDOM(0, sizeof(base64url_chars) - 1)];
	}
	return s;
}

}
