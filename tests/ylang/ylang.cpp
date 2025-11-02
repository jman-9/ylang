#include "ylang.h"
#include "core/Scanner.h"
#include "core/Parser.h"
#include "core/SemanticAnalyzer.h"
#include "core/BytecodeBuilder.h"
#include "core/vm/Machine.h"
#include <iostream>
#include <format>
#include <filesystem>
#include <fstream>
using namespace std;


bool ylang::Run(const string& srcPath)
{
	filesystem::path path{srcPath};
	string srcName = path.filename().string();

	ifstream ifs(srcPath, ios::binary);
	if (!ifs.is_open()) {
		cout << format("failed to open file: {}\n", srcPath);
		return false;
	}
	string src((istreambuf_iterator<char>(ifs)), {});

	Scanner s;
	s.Scan(src);
	for(auto t : s._tokens)
	{
		cout << format("line:{},kind:{},val:{}\n", t.line, (int)t.kind, t.val);
	}

	cout << endl;
	if(!s._errors.empty())
	{
		for(auto e : s._errors)
		{
			cout << format("{}({}): error E{}: {}\n", srcPath, e.line, (int)e.code, e.msg);
		}
		return 1;
	}


	Parser p(s._tokens);
	TreeNode* ast = p.Parse();
	if(!ast)
	{//TODO
		throw 'n';
	}

	SemanticAnalyzer sa(*ast);
	if(!sa.Analyze())
	{//TODO
		throw 'n';
	}

	BytecodeBuilder bb(*ast);
	Bytecode c;
	if(!bb.Build(c))
	{//TODO
		throw 'n';
	}

	yvm::Machine m;
	m.Run(c);
	return true;
}
