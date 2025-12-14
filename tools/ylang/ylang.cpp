#include "ylang.h"
#include "ylangDefs.h"
#include "compiler/Scanner.h"
#include "compiler/StringInterpolator.h"
#include "compiler/Parser.h"
#include "compiler/SemanticAnalyzer.h"
#include "compiler/BytecodeBuilder.h"
#include "compiler/Compiler.h"
#include "vm/Machine.h"
#include <iostream>
#include <format>
using namespace std;


bool ylang::RunCode(const std::string& src)
{
	ycom::ErrorTable errTbl;
	ycom::Compiler cmplr;

	Program p;
	errTbl = cmplr.CompileCode(src, p);
	if(!errTbl.empty())
	{
		cout << endl;
		for(auto& [f, es] : errTbl)
		{
			for(auto& e : es)
				cout << format("{}({}): error E{}: {}\n", f, e.line, (int)e.code, e.msg);
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
	ycom::ErrorTable errTbl;
	ycom::Compiler cmplr;

	Program p;
	errTbl = cmplr.CompileFile(srcPath, p);
	if(!errTbl.empty())
	{
		cout << endl;
		for(auto& [f, es] : errTbl)
		{
			for(auto& e : es)
				cout << format("{}({}): error E{}: {}\n", f, e.line, (int)e.code, e.msg);
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
	cout << format("ylang {}\n\n", YLANG_VER);

	ycom::SemanticAnalyzer sa;
	ycom::BytecodeBuilder bb;
	yvm::Machine replMachine;
	Program prg;
	bool first = true;

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
		do {
			ycom::Scanner s;
			s.Scan(code);
			if(s._tokens.empty() || !s._errors.empty())
			{
				errs.insert(errs.end(), s._errors.begin(), s._errors.end());
				break;
			}

			ycom::StringInterpolator si;
			vector<Token> processed;
			for(auto& t : s._tokens)
			{
				if(t != EToken::Str)
				{
					processed.push_back(t);
					continue;
				}

				auto interpolated = si.Interpolate(t);
				if(!interpolated.errs.empty())
				{
					errs.insert(errs.end(), interpolated.errs.begin(), interpolated.errs.end());
					break;
				}
				processed.insert(processed.end(), interpolated.res.begin(), interpolated.res.end());
			}

			ycom::Parser p(processed);
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

			if(!bb.Build(*ast, prg))
			{
				errs.push_back(ErrorBuilder::Default(0, "bytecode build error"));
				break;
			}
			if(first)
			{
				first = false;
				replMachine.Run(prg, pc);
			}
			else
				replMachine.Continue(pc);
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
