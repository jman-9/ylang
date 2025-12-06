#include "util/Args.h"
#include "compiler/Compiler.h"
#include "vm/Machine.h"
using namespace std;


int main(int argc, const char** argv)
{
	yrun::ArgsCollector::Collect(argc, argv);

	ycom::Compiler cmplr;
	Program prg;
	//auto errs = cmplr.CompileFile("test.y", prg);
	auto errs = cmplr.CompileFile("../../examples/maze_gen_find.y", prg);
	//auto errs = cmplr.CompileFile("../../examples/rpg_sim.y", prg);
	//auto errs = cmplr.CompileFile("../../examples/langton_ant.y", prg);
	if(!errs.empty())
		return 1;

	yvm::Machine m;
	return m.Run(prg);
}
