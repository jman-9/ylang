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
	cout << "ylang 0.0.2\n\n";

	SemanticAnalyzer sa;
	BytecodeBuilder bb;
	yvm::Machine replMachine;

	vector<string> lines;

	char buf[4096];
	string code;
	int pc = 0;
	bool complete = true;
	for( ; ; )
	{
		code = "";

		if(complete)
		{
			lines.clear();
		}

		cout << (complete ? ">> " : ".. ");
		buf[0] = '\0';
		cin.getline(buf, 1024);
		if(buf[0] == '\0')
		{
			if(complete)
				continue;
			for(auto& l : lines)
			{
				code += l + '\n';
			}
		}
		else
		{
			if(string(buf) == "exit")
				break;

			lines.push_back(buf);
			if(complete)
			{
				code = buf;
			}
			else
			{
				continue;
			}
		}

		vector<Error> errs;

		bool run = false;
		do {	//todo memory leak
			Scanner s;
			s.Scan(code);
			if(s._tokens.empty() || !s._errors.empty())
			{
				errs.insert(errs.end(), s._errors.begin(), s._errors.end());
				s._errors.clear();
				break;
			}

			Parser p(s._tokens);
			TreeNode* ast = p.Parse();
			if(!ast || !p._errors.empty())
			{
				errs.insert(errs.end(), p._errors.begin(), p._errors.end());
				p._errors.clear();
				break;
			}

			if(!sa.Analyze(*ast))
			{
				errs.insert(errs.end(), sa._errors.begin(), sa._errors.end());
				sa._errors.clear();
				break;
			}

			Bytecode c;
			if(!bb.Build(*ast, c)) throw 'n';
			replMachine.Run(c, pc);
			pc = c._code.size();
			run = true;
		} while(0);

		if(!errs.empty())
		{
			if(complete)
			{
				for(auto e : errs)
				{
					if(e.IsIncompleteError())
					{
						complete = false;
						break;
					}
				}
				if(!complete)
					continue;
			}

			cout << endl;
			for(auto e : errs)
			{
				cout << format("{}: error E{}: {}\n", e.line, (int)e.code, e.msg);
			}
			cout << endl;
			complete = true;
		}
		else
		{
			complete = run;
		}
	}

	return true;
}
