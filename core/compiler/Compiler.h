#pragma once
#include "contract/Error.h"
#include "contract/Program.h"
#include "TreeNode.h"
#include <string>
#include <vector>
#include <map>


namespace ycom
{
using ErrorTable = std::map<std::string, std::vector<Error>>;

class Compiler
{
public:
	Compiler();

	void AddPath(std::string path);

	ErrorTable CompileCode(const std::string& src, Program& retProgram, const std::string& srcAbsPath = "");
	ErrorTable CompileFile(const std::string& srcPath, Program& retProgram);

protected:
	std::vector<std::string> _paths;

	ErrorTable CompileCodePriv(const std::string& src, Program& retProgram, const std::string& srcAbsPath = "");
	ErrorTable CompileFilePriv(const std::string& srcAbsPath, Program& retProgram);

	std::vector<Error> ParseCode(const std::string& src, TreeNodeSptr& retAstRoot);
	std::vector<Error> ParseFile(const std::string& srcPath, TreeNodeSptr& retAstRoot);

	std::vector<Error> ExtractIncludes(const TreeNode& root, std::vector<TreeNodeSptr>& retIncludes);

	std::vector<Error> ReadSourceFile(const std::string& srcPath, std::string& retSrc) const;
};

}
