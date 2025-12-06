#include "ylang.h"
#include "compiler/Scanner.h"
#include "compiler/StringInterpolator.h"
#include "compiler/Parser.h"
#include "compiler/SemanticAnalyzer.h"
#include "compiler/BytecodeBuilder.h"
#include "compiler/Compiler.h"
#include "vm/Machine.h"
#include <iostream>
#include <format>
#include <filesystem>
#include <fstream>
using namespace std;


bool ylang::RunCode(const std::string& src)
{
	vector<Error> errs;
	ycom::Compiler cmplr;

	Program p;
	errs = cmplr.CompileCode(src, p);
	if(!errs.empty())
	{
		cout << endl;
		for(auto e : errs)
		{
			cout << format("{}({}): error E{}: {}\n", "code", e.line, (int)e.code, e.msg);
		}
		cout << endl;
		return false;
	}

	yvm::Machine m;
	m.Run(p);
	return true;
}

bool ylang::RunFile(const string& srcPath)
{
	vector<Error> errs;
	ycom::Compiler cmplr;

	Program p;
	errs = cmplr.CompileFile(srcPath, p);
	if(!errs.empty())
	{
		cout << endl;
		for(auto e : errs)
		{
			cout << format("{}({}): error E{}: {}\n", srcPath, e.line, (int)e.code, e.msg);
		}
		cout << endl;
		return false;
	}

	yvm::Machine m;
	m.Run(p);
	return true;
}

bool ylang::StartRepl()
{
	cout << "ylang 0.0.5\n\n";

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

			Program prg;
			if(!bb.Build(*ast, prg)) throw 'n';
			replMachine.Run(prg, pc);
			pc = (int)prg._mainCode._code.size();
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
