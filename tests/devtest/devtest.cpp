#include "core/Scanner.h"
#include "core/StringInterpolator.h"
#include "core/Parser.h"
#include "core/SemanticAnalyzer.h"
#include "core/BytecodeBuilder.h"
#include "core/vm/Machine.h"
#include "core/Args.h"
#include <iostream>
#include <format>
using namespace std;

#define TOKEN_DEBUG_OUT
#define BYTECODE_DEBUG_OUT


const char* testcode1 = R"TEST(
//a = 2; b = a++ + a++; println("{b} {a}");

//include sys;
//println(sys.version);
/*
a = {};
a["hhuu"] = 10;
println(a);
println(a.len());
println(a.keys());
println(a.values());
println(a.items());
*/

class Test
{
	a = 1;
	c = "haha";
	l = [];
	d = {};
};

t = Test();
t.a = "huhu";
t.c = 19;
t.l.append(1092);
t.l.append("haha");
t.d["humm"] = 15;;
t.d["humm"] = "check";
println(t);
println(t.a);
println(t.c);
println(t.l);
println(t.d);

)TEST";
const char* testcode2 = R"TEST(
)TEST";
const char* testcode3 = R"TEST(
)TEST";

int main(int argc, const char** argv)
{
	yrun::ArgsCollector::Collect(argc, argv);

	Scanner s;
	//s.Scan(testcode);
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
