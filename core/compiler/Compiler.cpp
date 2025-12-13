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

ErrorTable Compiler::CompileCode(const string& src, Program& retProgram)
{
	ErrorTable errTbl;
	vector<Error> errs;
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
			errs = ParseCode(curSrc, ast);
			if(!errs.empty())
			{
				errTbl[ curMod ] = errs;
				break;
			}

			ast->self.val = curMod;

			incStack.push({});
			errs = ExtractIncludes(*ast, incStack.top().first);
			if(!errs.empty())
			{
				errTbl[ curMod ].insert(errTbl[ curMod ].end(), errs.begin(), errs.end());
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
			errs = ReadSourceFile(resInc.absPath + ".y", curSrc);
			if(!errs.empty())
			{
				errTbl[ curMod ].insert(errTbl[ curMod ].end(), errs.begin(), errs.end());
				break;
			}
			if(!curSrc.empty())
			{
				curMod = resInc.absPath;
				break;
			}
		}
		if(!errs.empty()) break;
	} while(!curSrc.empty());

	unordered_map<std::string, Program> prgMap;
	if(errTbl.empty())
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
				errTbl[ast->self.val] = sa._errors;
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

	if(!errTbl.empty())
	{
	#ifdef ERROR_DEBUG_OUT
		for(auto& [f, es] : errTbl)
		{
			for(auto& e : es)
			{
				cout << format("{}({}): error E{}: {}\n", f, e.line, (int)e.code, e.msg);
			}
		}
	#endif
		return errTbl;
	}

	retProgram = prgMap[ MAIN_PRG ];
	retProgram._programTable = prgMap;
	retProgram._programTable.erase(MAIN_PRG);
	return {};
}

ErrorTable Compiler::CompileFile(const string& srcPath, Program& retProgram)
{
	auto fsPath = filesystem::path{srcPath};
	string base = fsPath.parent_path().string();
	if(!base.empty())
		filesystem::current_path(base);

	string src;
	auto errs = ReadSourceFile(fsPath.filename().string(), src);
	if(!errs.empty())
	{
		ErrorTable errTbl;
		errTbl[srcPath] = errs;
		return errTbl;
	}
	auto errTbl = CompileCode(src, retProgram);
	if(!errTbl.empty()) return errTbl;

	retProgram._name = fsPath.stem().string();
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
			if(!interpolated.errs.empty())
			{
				errs.insert(errs.end(), interpolated.errs.begin(), interpolated.errs.end());
				break;
			}
			processed.insert(processed.end(), interpolated.res.begin(), interpolated.res.end());
		}
		if(!errs.empty()) break;

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
