#include "Compiler.h"
#include "Scanner.h"
#include "StringInterpolator.h"
#include "Parser.h"
#include "SemanticAnalyzer.h"
#include "BytecodeBuilder.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>
using namespace std;


namespace ycom
{

#if _DEBUG || DEBUG
#define TOKEN_DEBUG_OUT
#define BYTECODE_DEBUG_OUT
#define ERROR_DEBUG_OUT
#endif

vector<Error> Compiler::CompileCode(const string& src, Program& retProgram)
{
	vector<Error> errs;

	do {
		Scanner s;
		s.Scan(src);
		if(!s._errors.empty())
		{
			errs.insert(errs.end(), s._errors.begin(), s._errors.end());
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

	#ifdef TOKEN_DEBUG_OUT
		for(auto t : s._tokens)
		{
			string ts = format("line:{},kind:{},val:{}", t.line, (int)t.kind, t.val);
			cout << ts << endl;
		}
	#endif

		Parser p(processed);
		auto ast = p.Parse();
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
		if(!bb.Build(*ast, retProgram))
		{//TODO trace
			throw 'n';
		}

	#ifdef BYTECODE_DEBUG_OUT
		for(int i=0; i<retProgram._mainCode._codeStr.size(); i++)
		{
			cout << format("{:4} {}\n", i, retProgram._mainCode._codeStr[i]);
		}
	#endif

	} while(0);

#ifdef ERROR_DEBUG_OUT
	for(auto e : errs)
	{
		string errStr = format("{}({}): error E{}: {}", "some file", e.line, (int)e.code, e.msg);
		cout << errStr << endl;
	}
#endif
	return errs;
}

vector<Error> Compiler::CompileFile(const std::string& srcPath, Program& retProgram)
{
	filesystem::path path{srcPath};
	string srcName = path.filename().string();

	ifstream ifs(srcPath, ios::binary);
	if (!ifs.is_open()) {
		vector<Error> e;
		e.push_back(ErrorBuilder::FileOpenError(0, srcPath));
		return e;
	}
	string src((istreambuf_iterator<char>(ifs)), {});
	return CompileFile(src, retProgram);
}


}
