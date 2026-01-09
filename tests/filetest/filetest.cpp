#include "util/Args.h"
#include "compiler/Compiler.h"
#include "vm/Machine.h"
using namespace std;


int main(int argc, const char** argv)
{
	yrun::ArgsCollector::Collect(argc, argv);

	ycom::Compiler cmplr;
	Program prg;
	//auto errs = cmplr.CompileFile("ys/test.y", prg);
	//auto errs = cmplr.CompileFile("../../examples/maze_gen_find.y", prg);
	//auto errs = cmplr.CompileFile("../../examples/rpg_sim.y", prg);
	//auto errs = cmplr.CompileFile("../../examples/langton_ant.y", prg);
	//auto errs = cmplr.CompileFile("../../examples/city_sim.y", prg);
	auto errs = cmplr.CompileFile("../../examples/grand_strategy/main.y", prg);
	//auto errs = cmplr.CompileFile("ys/app.y", prg);
	//auto errs = cmplr.CompileFile("ys/ss.y", prg);
//	auto errs = cmplr.CompileFile("ys/cst.y", prg);
	//auto errs = cmplr.CompileFile("ys/xdef_stage19_partial_19-4/xdef.y", prg);
	//auto errs = cmplr.CompileFile("ys/ai_dungeon_fixed/main.y", prg);
	//auto errs = cmplr.CompileFile("ys/vm_runtime_game_ops_extra/main.y", prg);
	//auto errs = cmplr.CompileFile("ys/project_line_counter.y", prg);

	if(!errs.empty())
		return 1;

	yvm::Machine m;
	m.Run(prg);
}
