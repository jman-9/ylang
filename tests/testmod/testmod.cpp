#include "ycontract.h"
#include <string>
#include <vector>
using namespace std;


extern "C"
{
	EXPORT YRet version(YArgs* args);
	EXPORT YRet platform(YArgs* args);

	EXPORT YRet ProcessUserData(YArgs* args);

}

EXPORT YRet version(YArgs* args)
{
	return {};
}

EXPORT YRet platform(YArgs* args)
{
	return {};
}


EXPORT YRet ProcessUserData(YArgs* args)
{
	string name = args->args[0].ToStr();
	int64_t age = args->args[1].ToInt64();
	vector<YArg> scores = args->args[2].ToList();

	YRet yr;
	yr.code = 0;

	double sum = 0;
	for(auto& ya : scores)
	{
		sum += ya.ToDouble();
	}

	if(!yr.single.FromDouble(sum /= scores.size()))
	{
		yr.code = 1;
	}

	return yr;
}
