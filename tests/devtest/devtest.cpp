#include "compiler/Compiler.h"
#include "vm/Machine.h"
#include "util/Args.h"
#include <iostream>
#include <format>
using namespace std;

const char* testcode1 =
R"TEST(
t = 10;

fn test(t1, t2)
		{
			a = t1;
			b = t2;

			return a * b;
		}
		a = test;
		println(a(98, 10));
)TEST";
const char* testcode2 = R"TEST(
)TEST";
const char* testcode3 = R"TEST(
)TEST";

int main(int argc, const char** argv)
{
	yrun::ArgsCollector::Collect(argc, argv);

	ycom::Compiler cmplr;
	Program prg;
	auto errs = cmplr.CompileCode(string(testcode1) + string(testcode2) + string(testcode3), prg);
	if(!errs.empty())
		return 1;

	yvm::Machine m;
	return m.Run(prg);
}
