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



static std::vector<Error> Build(const std::string& src, Bytecode& retBytecode)
{
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

		SemanticAnalyzer sa;
		sa.Analyze(*ast);
		if(!sa._errors.empty())
		{
			errs.insert(errs.end(), sa._errors.begin(), sa._errors.end());
			break;
		}

		BytecodeBuilder bb;
		if(!bb.Build(*ast, retBytecode))
		{//TODO trace
			throw 'n';
		}
	} while(0);

	return errs;
}


static std::vector<Error> Run(const std::string& src)
{
	vector<Error> errs;

	Bytecode c;
	errs = Build(src, c);
	if(!errs.empty())
		return errs;

	yvm::Machine m;
	m.Run(c);
	return errs;
}

bool ylang::RunCode(const std::string& src)
{
	auto errs = Run(src);
	if(!errs.empty())
	{
		cout << endl;
		for(auto e : errs)
		{
			cout << format("{}({}): error E{}: {}\n", "code", e.line, (int)e.code, e.msg);
		}
		cout << endl;
	}
	return errs.empty();
}

bool ylang::RunFile(const string& srcPath)
{
	filesystem::path path{srcPath};
	string srcName = path.filename().string();

	ifstream ifs(srcPath, ios::binary);
	if (!ifs.is_open()) {
		cout << format("failed to open file: {}\n", srcPath);
		return false;
	}
	string src((istreambuf_iterator<char>(ifs)), {});
	auto errs = Run(src);

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

bool ylang::StartRepl()
{
	cout << "ylang 0.0.1\n";

	SemanticAnalyzer sa;
	BytecodeBuilder bb;
	yvm::Machine replMachine;

	char line[4096];
	int pc = 0;
	for( ; ; )
	{
		cout << ">> ";
		cin.getline(line, 1024);

		vector<Error> errs;

		do {	//todo memory leak
			Scanner s;
			s.Scan(line);
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

			if(!sa.Analyze(*ast))
			{
				errs.insert(errs.end(), p._errors.begin(), p._errors.end());
				break;
			}

			Bytecode c;
			if(!bb.Build(*ast, c)) throw 'n';
			replMachine.Run(c, pc);
			pc = c._code.size();
		} while(0);
	}
}
