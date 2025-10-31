#include "ylang.h"
#include "core/Scanner.h"
#include "core/Parser.h"
#include "core/SemanticAnalyzer.h"
#include "core/BytecodeBuilder.h"
#include "core/vm/Machine.h"
#include <format>
using namespace std;

int main()
{
	Scanner s2;

	for(auto t : s2._tokens)
	{
		string ts = format("line:{},kind:{},val:{}", t.line, (int)t.kind, t.val);
		cout << ts << endl;
	}

	if(!s2._errors.empty())
	{
		for(auto e : s2._errors)
		{
			string errStr = format("{}({}): error E{}: {}", "some file", e.line, (int)e.code, e.msg);
			cout << errStr << endl;
		}
	}
	else
	{
		Parser p(s2._tokens);
		TreeNode* ast = p.Parse();
		if(!ast) throw 'n';

		SemanticAnalyzer sa(*ast);
		if(!sa.Analyze()) throw 'n';

		BytecodeBuilder bb(*ast);
		Bytecode c;
		if(!bb.Build(c)) throw 'n';

		yvm::Machine m;
		m.Run(c);
	}

	return 0;
}
