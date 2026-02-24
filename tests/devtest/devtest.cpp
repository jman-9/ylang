#include "compiler/Compiler.h"
#include "vm/Machine.h"
#include "util/Args.h"
#include <iostream>
#include <format>
using namespace std;

const char* testcode1 =
R"TEST(

fn outer_through(z) return z;

class Test
{
	f = 15;
	fn Test(fv) { f = fv; }

	fn ttt() { return f; }

	fn test(t1)
	{
		fn inner_through(tt) { return tt; }

		fn cap(d)
		{
			return outer_through( inner_through( ttt() ) ) + d;
		}

		return cap;
	}
}

t = Test(98);
c = t.test(9);
d = c(2);
println(d);

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
