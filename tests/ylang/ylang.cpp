#include "ylang.h"
#include "core/Scanner.h"
#include "core/Parser.h"
#include "core/SemanticAnalyzer.h"
#include "core/BytecodeBuilder.h"
#include "core/vm/Machine.h"
#include <format>
#include <filesystem>
#include <fstream>
using namespace std;

int main(int argc, const char** argv)
{
	if(argc < 2)
	{
		string toolName = filesystem::path{argv[0]}.filename().string();

		cout << format("{} <source file>\n", toolName);
		cout << "\n";
		cout << format("ex) {} perfect.y\n", toolName);
		return 1;
	}

	filesystem::path path{argv[1]};
	string srcName = path.filename().string();
	string srcPath = path.string();

	std::ifstream ifs(argv[1], std::ios::binary);
	if (!ifs.is_open()) {
		cout << format("failed to open file: {}\n", srcPath);
		return 1;
	}
	std::string src((std::istreambuf_iterator<char>(ifs)), {});

	Scanner s;
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
	return 0;
}
