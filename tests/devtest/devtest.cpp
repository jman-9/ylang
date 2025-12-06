#include "compiler/Scanner.h"
#include "compiler/StringInterpolator.h"
#include "compiler/Parser.h"
#include "compiler/SemanticAnalyzer.h"
#include "compiler/BytecodeBuilder.h"
#include "vm/Machine.h"
#include "util/Args.h"
#include <iostream>
#include <format>
using namespace std;

#define TOKEN_DEBUG_OUT
#define BYTECODE_DEBUG_OUT

const char* testcode1 = R"TEST(
a = [0];
for(i=0; i<1000000; i++) {
	a.append(10);
	//a.pop_front();
}

println("{a.len()} pop start");
for(i=0; i<1000000; i++) {
	a.pop_back();
}
println("{a.len()} pop end");
)TEST";
const char* testcode2 = R"TEST(
)TEST";
const char* testcode3 = R"TEST(
)TEST";

int main(int argc, const char** argv)
{
	yrun::ArgsCollector::Collect(argc, argv);

	Scanner s;
	s.Scan(string(testcode1) + string(testcode2) + string(testcode3));

#ifdef TOKEN_DEBUG_OUT
	for(auto t : s._tokens)
	{
		string ts = format("line:{},kind:{},val:{}", t.line, (int)t.kind, t.val);
		cout << ts << endl;
	}
#endif

	if(!s._errors.empty())
	{
		for(auto e : s._errors)
		{
			string errStr = format("{}({}): error E{}: {}", "some file", e.line, (int)e.code, e.msg);
			cout << errStr << endl;
		}
	}
	else
	{
		StringInterpolator si;
		vector<Token> processed;
		for(auto& t : s._tokens)
		{
			if(t != EToken::Str)
			{
				processed.push_back(t);
				continue;
			}

			auto interpolated = si.Interpolate(t);
			if(interpolated.empty())
			{//todo error
				throw 'n';
			}
			processed.insert(processed.end(), interpolated.begin(), interpolated.end());
		}

		Parser p(processed);
		auto ast = p.Parse();
		if(!ast)
		{
			for(auto e : p._errors)
			{
				string errStr = format("{}({}): error E{}: {}", "some file", e.line, (int)e.code, e.msg);
				cout << errStr << endl;
			}
			return 1;
		}

		SemanticAnalyzer sa;
		if(!sa.Analyze(*ast))
		{
			for(auto e : sa._errors)
			{
				string errStr = format("{}({}): error E{}: {}", "some file", e.line, (int)e.code, e.msg);
				cout << errStr << endl;
			}
			return 1;
		}

		BytecodeBuilder bb;
		Program prg;
		if(!bb.Build(*ast, prg)) throw 'n';

	#ifdef BYTECODE_DEBUG_OUT
		for(int i=0; i<prg._mainCode._codeStr.size(); i++)
		{
			cout << format("{:4} {}\n", i, prg._mainCode._codeStr[i]);
		}
	#endif

		yvm::Machine m;
		m.Run(prg);
	}

	return 0;
}
