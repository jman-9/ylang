#include "Compiler.h"
#include "Scanner.h"
#include "StringInterpolator.h"
#include "Parser.h"
#include "SemanticAnalyzer.h"
#include "BytecodeBuilder.h"
#include "NamespaceUtil.h"
#include "builtin/BuiltinGarage.h"
#include "util/StrUtil.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stack>
using namespace std;


namespace ycom
{

#if _DEBUG || DEBUG
#define TOKEN_DEBUG_OUT
#define BYTECODE_DEBUG_OUT
#define ERROR_DEBUG_OUT
#endif

#define MAIN_PRG "_____main_____"

vector<Error> Compiler::CompileCode(const string& src, Program& retProgram)
{
	vector<Error> totalErrs;
	vector<Error> curErrs;
	vector<TreeNodeSptr> includes;
	stack<pair<vector<TreeNodeSptr>, int>> incStack;
	unordered_map<string, TreeNodeSptr> astMap;
	stack<TreeNodeSptr> astStack;

	string curSrc = src;
	string curMod = MAIN_PRG;

	do {
		if(!curSrc.empty())
		{
			auto inserted = astMap.insert({curMod, {}});
			auto& ast = inserted.first->second;
			totalErrs = ParseCode(curSrc, ast);
			if(!totalErrs.empty()) break;

			ast->self.val = curMod;

			incStack.push({});
			curErrs = ExtractIncludes(*ast, incStack.top().first);
			if(!curErrs.empty())
			{
				totalErrs.insert(totalErrs.end(), curErrs.begin(), curErrs.end());
				break;
			}
			astStack.push(ast);
		}

		curSrc = "";
		for( ; !incStack.empty(); )
		{
			auto& incStmts = incStack.top().first;
			auto& idx = incStack.top().second;

			if(idx >= incStmts.size())
			{
				incStack.pop();
				continue;
			}

			auto& stmt = incStmts[idx++];
			auto& incName = stmt->childs.front()->self.val;
			if(ybuiltin::Garage::IsBuiltin(incName)) continue;

			auto resInc = NamespaceUtil::ResolveInclude(incName);
			//TODO qaz mod check
			curErrs = ReadSourceFile(resInc.absPath + ".y", curSrc);
			if(!curErrs.empty())
			{
				totalErrs.insert(totalErrs.end(), curErrs.begin(), curErrs.end());
				break;
			}
			if(!curSrc.empty())
			{
				curMod = resInc.absPath;
				break;
			}
		}
		if(!curErrs.empty()) break;
	} while(!curSrc.empty());

	unordered_map<std::string, Program> prgMap;
	if(totalErrs.empty())
	{
		while(!astStack.empty())
		{
			Program prg;

			auto ast = astStack.top();
			astStack.pop();

			if(prgMap.contains(ast->self.val))
				continue;

			SemanticAnalyzer sa;
			sa.Analyze(*ast);
			if(!sa._errors.empty())
			{
				totalErrs.insert(totalErrs.end(), sa._errors.begin(), sa._errors.end());
				break;
			}

			BytecodeBuilder bb;
			if(!bb.Build(*ast, prg, &prgMap))
			{//TODO trace
				throw 'n';
			}

			prg._name = filesystem::path(ast->self.val).stem().string();
			prgMap[ ast->self.val ] = prg;

		#ifdef BYTECODE_DEBUG_OUT
			cout << prg._name << endl;
			for(int i=0; i<prg._mainCode._codeStr.size(); i++)
			{
				cout << format("{:4} {}\n", i, prg._mainCode._codeStr[i]);
			}
			cout << endl;
		#endif
		}
	}

	if(!totalErrs.empty())
	{
	#ifdef ERROR_DEBUG_OUT
		for(auto e : totalErrs)
		{
			string errStr = format("{}({}): error E{}: {}", "some file", e.line, (int)e.code, e.msg);
			cout << errStr << endl;
		}
	#endif
		return totalErrs;
	}

	retProgram = prgMap[ MAIN_PRG ];
	retProgram._programTable = prgMap;
	retProgram._programTable.erase(MAIN_PRG);
	return {};
}

vector<Error> Compiler::CompileFile(const string& srcPath, Program& retProgram)
{
	string src;
	auto errs = ReadSourceFile(srcPath, src);
	if(!errs.empty()) return errs;
	errs = CompileCode(src, retProgram);
	if(!errs.empty()) return errs;

	retProgram._name = filesystem::path(srcPath).stem().string();
	return {};
}


vector<Error> Compiler::ParseCode(const string& src, TreeNodeSptr& retAstRoot)
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
		retAstRoot = ast;
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

vector<Error> Compiler::ParseFile(const std::string& srcPath, TreeNodeSptr& retAstRoot)
{
	string src;
	auto errs = ReadSourceFile(srcPath, src);
	if(!errs.empty()) return errs;
	return ParseCode(src, retAstRoot);
}

vector<Error> Compiler::ExtractIncludes(const TreeNode& root, vector<TreeNodeSptr>& retIncludes)
{
	for(auto& stmt : root.childs)
	{
		if(stmt->self != EToken::Include)
			continue;

		//TODO
		cout << stmt->childs.front()->self.val << endl;
		retIncludes.push_back(stmt);
	}
	return {};
}

vector<Error> Compiler::ReadSourceFile(const std::string& srcPath, std::string& retSrc) const
{
	filesystem::path path{srcPath};
	string srcName = path.filename().string();

	ifstream ifs(srcPath, ios::binary);
	if (!ifs.is_open()) {
		vector<Error> e;
		e.push_back(ErrorBuilder::FileOpenError(0, srcPath));
		return e;
	}
	retSrc = string((istreambuf_iterator<char>(ifs)), {});
	return {};
}

}
