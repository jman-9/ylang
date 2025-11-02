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

	vector<Error> errs;

	do {	//todo memory leak
		Scanner s;
		s.Scan(src);
		if(!s._errors.empty())
		{
			errs.insert(errs.end(), s._errors.begin(), s._errors.end());
			break;
		}

		Parser p(s._tokens);
		TreeNode* ast = p.Parse();
		if(!p._errors.empty())
		{
			errs.insert(errs.end(), p._errors.begin(), p._errors.end());
			break;
		}

		SemanticAnalyzer sa(*ast);
		sa.Analyze();
		if(!sa._errors.empty())
		{
			errs.insert(errs.end(), sa._errors.begin(), sa._errors.end());
			break;
		}

		BytecodeBuilder bb(*ast);
		Bytecode c;
		if(!bb.Build(c))
		{//TODO trace
			throw 'n';
		}

		yvm::Machine m;
		m.Run(c);
	} while(0);

	if(!errs.empty())
	{
		cout << endl;
		for(auto e : errs)
		{
			cout << format("{}({}): error E{}: {}\n", srcPath, e.line, (int)e.code, e.msg);
		}
		cout << endl;
	}
	return errs.empty();
}
