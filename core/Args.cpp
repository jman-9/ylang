#pragma once
#include "Args.h"


namespace yrun
{

Args::Args() {}
Args::~Args() {}

Args& Args::I()
{
	static Args s_args;
	return s_args;
}

void ArgsCollector::Collect(int argc, char** argv)
{
	g_Args.clear();
	for(int i=0; i<argc; i++)
	{
		g_Args.push_back(argv[i]);
	}
}

}
