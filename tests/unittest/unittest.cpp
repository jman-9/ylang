#include "catch2/catch_amalgamated.hpp"
#include "module/ModuleManager.h"
#include <iostream>
#include <map>
#include <vector>
using namespace std;


TEST_CASE( "ModuleManager Test", "[modmgr]" )
{

}

static const Catch::LeakDetector leakDetector;

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;
	(void)&leakDetector;

	static Catch::Session _session;
	Catch::ConfigData& cfg = _session.configData();

	cfg.showSuccessfulTests = true;
	cfg.testsOrTags.push_back("[modmgr]");

	int numFailed = _session.run();
};
