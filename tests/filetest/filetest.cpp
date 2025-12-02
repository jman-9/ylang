#include "core/Args.h"
#include "../../tools/ylang/ylang.h"
using namespace std;

int main(int argc, const char** argv)
{
	yrun::ArgsCollector::Collect(argc, argv);

	ylang y;
	y.RunFile("test.y");

	return 0;
}
