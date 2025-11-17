#pragma once
#include <stdint.h>
#include <string>
#include <vector>


namespace yrun
{

class Args : public std::vector<std::string>
{
private:
	Args();
public:
	~Args();
	static Args& I();
};


struct ArgsCollector
{
	static void Collect(int argc, const char** argv);
};

}

#define g_Args (yrun::Args::I())
