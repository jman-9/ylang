add_library(ycore STATIC
	Scanner.cpp
	Scanner.h
  "Token.h" "Token.cpp" "Error.h" "Error.cpp" "Parser.h" "Parser.cpp" "SemanticAnalyzer.h" "SemanticAnalyzer.cpp" "TreeNode.h" "BytecodeBuilder.h" "BytecodeBuilder.cpp" "Symbol.h" "vm/Machine.cpp" "vm/yvm.h"  "InstructionSet.h" "Instruction.h" "vm/Machine.h" "Bytecode.h" "Instruction.inl" "vm/Variable.h" "vm/Variable.cpp" "StringInterpolator.h" "StringInterpolator.cpp"   "contract/ycontract.h" "contract/ycontract.inl")

target_include_directories(ycore
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/contract
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vm
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
