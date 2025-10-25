add_library(ylang_core STATIC
	Scanner.cpp
	Scanner.h
  "Token.h" "Token.cpp" "Error.h" "Error.cpp" "Parser.h" "Parser.cpp" "SemanticAnalyzer.h" "SemanticAnalyzer.cpp" "TreeNode.h" "BytecodeBuilder.h" "BytecodeBuilder.cpp" "Symbol.h")

target_include_directories(ylang_core
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
