#include "util/Args.h"
#include "../../tools/ylang/ylang.h"
#include <iostream>
using namespace std;


int main(int argc, const char** argv)
{
	yrun::ArgsCollector::Collect(argc, argv);

	ylang y;
	//y.RunFile("test.y");
	y.RunFile("../../examples/maze_gen_find.y");
	//y.RunFile("../../examples/rpg_sim.y");
	//y.RunFile("../../examples/langton_ant.y");

	return 0;
}
