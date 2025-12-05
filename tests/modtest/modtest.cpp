#include "ycontract.h"
#include "Variable.h"
#include <iostream>
#include <map>
#include <vector>
using namespace std;
using namespace yvm;

int main()
{
	TCHAR t[1024];
	GetCurrentDirectory(1024, t);

	string s = t;
	s += "/../testmod/testmod.dll";

	void* handle = LOAD_LIB(s.c_str());
	if (!handle) {
	#if defined(_WIN32)
		std::cerr << "Failed to load DLL.\n";
	#else
		std::cerr << "Failed to load SO: " << dlerror() << "\n";
	#endif
		return 1;
	}

	vector<string> fnList = { "version", "platform", "ProcessUserData" };
	map<string, YModFn> fnMap;

	for(auto& fname : fnList)
	{
		fnMap[fname] = (YModFn)LOAD_SYM(handle, fname.c_str());
		if(!fnMap[fname])
		{
			std::cerr << "Missing symbol " << fname << "\n";
			CLOSE_LIB(handle);
			return 1;
		}
	}

	Variable name;		name.SetStr("Lucia");
	Variable age;		age.SetInt(1);
	Variable scores;	scores.SetList();

	scores.list().resize(3);
	scores.list()[0].SetFloat(75.0);
	scores.list()[1].SetFloat(45.0);
	scores.list()[2].SetFloat(65.0);

	YArgs args;
	args.Reset(3);
	args.args[0] = name.ToContract();
	args.args[1] = age.ToContract();
	args.args[2] = scores.ToContract();

	auto d = fnMap[fnList[2]](&args);
	double avg = d.single.ToDouble();

	Variable vavg;
	vavg.SetFloat(avg);
	cout << vavg.ToStr() << '\n';
};
