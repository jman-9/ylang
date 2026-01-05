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
//#define TOKEN_DEBUG_OUT
#define BYTECODE_DEBUG_OUT
#define ERROR_DEBUG_OUT
#endif

#define MAIN_PRG "_____main_____"


Compiler::Compiler()
{
	_paths.push_back(filesystem::current_path().string());
}

void Compiler::AddPath(std::string path)
{
	_paths.push_back(path);
}

ErrorTable Compiler::CompileCode(const string& src, Program& retProgram, const string& srcAbsPath /* = "" */)
{
	auto errTbl = CompileCodePriv(src, retProgram, srcAbsPath);
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

	string name = srcAbsPath.empty() ? MAIN_PRG : srcAbsPath;
	Program prg = retProgram._programTable[ name ];
	prg._programTable = retProgram._programTable;
	prg._programTable.erase(name);
	retProgram = prg;
	retProgram._name = filesystem::path(srcAbsPath).stem().string();
	return {};
}

ErrorTable Compiler::CompileFile(const string& srcPath, Program& retProgram)
{
	auto fsPath = filesystem::path{srcPath};
	filesystem::path absPath = filesystem::absolute(fsPath);
	if(!filesystem::exists(absPath))
	{
		ErrorTable errTbl;
		errTbl[absPath.string()].push_back(ErrorBuilder::FileOpenError(0, srcPath));
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

	_paths.push_back(absPath.parent_path().string());

	auto errTbl = CompileFilePriv(absPath.string(), retProgram);
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
		_paths.pop_back();
		return errTbl;
	}

	_paths.pop_back();

	string modPath = absPath.string();
	if(modPath.ends_with(".y"))
		modPath.resize(modPath.size() - 2);

	Program prg = retProgram._programTable[ modPath ];
	prg._programTable = retProgram._programTable;
	prg._programTable.erase(modPath);
	retProgram = prg;
	retProgram._name = fsPath.stem().string();
	return {};
}

ErrorTable Compiler::CompileCodePriv(const string& src, Program& retProgram, const string& srcAbsPath /*= ""*/)
{
	ErrorTable errTbl;
	vector<Error> errs;
	vector<TreeNodeSptr> includes;

	string curSrc = src;
	string curMod = srcAbsPath.empty() ? MAIN_PRG : srcAbsPath;
	if(curMod.ends_with(".y"))
		curMod.resize(curMod.size() - 2);
	string curModDir = srcAbsPath.empty() ? filesystem::current_path().string() : filesystem::path(srcAbsPath).parent_path().string();

	TreeNodeSptr ast;
	errs = ParseCode(curSrc, ast);
	if(!errs.empty())
	{
		errTbl[ curMod ] = errs;
		return errTbl;
	}

	ast->self.val = curMod;

	errs = ExtractIncludes(*ast, includes);
	if(!errs.empty())
	{
		errTbl[ curMod ].insert(errTbl[ curMod ].end(), errs.begin(), errs.end());
		return errTbl;
	}

	vector<string> paths = _paths;
	paths.push_back(curModDir);

	for(auto& inc : includes)
	{
		auto& incName = inc->childs.front()->self.val;
		if(ybuiltin::Garage::IsBuiltin(incName)) continue;

		NamespaceUtil::Resolution resInc;
		for(auto base : paths)
		{
			resInc = NamespaceUtil::ResolveInclude(incName, base);
			if(filesystem::exists(resInc.absPath + ".y"))
				break;
		}
		if(retProgram._programTable.contains(resInc.absPath))
			continue;

		//TODO qaz mod check
		errTbl = CompileFilePriv(resInc.absPath + ".y", retProgram);
		if(!errTbl.empty())
			return errTbl;
	}

	SemanticAnalyzer sa;
	sa.Analyze(*ast, paths);
	if(!sa._errors.empty())
	{
		errTbl[ curMod ].insert(errTbl[ curMod ].end(), errs.begin(), errs.end());
		return errTbl;
	}

	Program prg;
	BytecodeBuilder bb;
	if(!bb.Build(*ast, prg, &retProgram._programTable, paths))
	{
		errTbl[ curMod ].push_back(ErrorBuilder::Default(0, "bytecode build error"));
		return errTbl;
	}

	prg._path = curMod;
	prg._name = filesystem::path(curMod).stem().string();
	retProgram._programTable[ curMod ] = prg;

#ifdef BYTECODE_DEBUG_OUT
	for(auto& c : prg._classTable)
	{
		cout << c.first << endl;
		for(auto& [k, v] : c.second._funcMap)
		{
			cout << k << endl;
			for(int i=0; i<c.second._funcs[v]._codeStrs.size(); i++)
			{
				cout << format("{:4} {}\n", i, c.second._funcs[v]._codeStrs[i]);
			}
		}
	}

	cout << prg._name << endl;
	for(int i=0; i<prg._mainCode._codeStrs.size(); i++)
	{
		cout << format("{:4} {}\n", i, prg._mainCode._codeStrs[i]);
	}
	cout << endl;
#endif

	return {};
}

ErrorTable Compiler::CompileFilePriv(const string & srcAbsPath, Program & retProgram)
{
	string src;
	auto errs = ReadSourceFile(srcAbsPath, src);
	if(!errs.empty())
	{
		ErrorTable errTbl;
		errTbl[srcAbsPath] = errs;
		return errTbl;
	}
	return CompileCodePriv(src, retProgram, srcAbsPath);
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
