add_library(ycore STATIC
	Scanner.cpp
	Scanner.h
  "Token.h" "Token.cpp" "Error.h" "Error.cpp" "Parser.h" "Parser.cpp" "SemanticAnalyzer.h" "SemanticAnalyzer.cpp" "TreeNode.h" "BytecodeBuilder.h" "BytecodeBuilder.cpp" "Symbol.h" "vm/Machine.cpp" "vm/yvm.h"  "InstructionSet.h" "Instruction.h" "vm/Machine.h" "Bytecode.h" "Instruction.inl" "vm/Variable.h" "vm/Variable.cpp" "StringInterpolator.h" "StringInterpolator.cpp"   "contract/ycontract.h" "contract/ycontract.inl" "module/ModuleManager.h" "module/ModuleManager.cpp"  "module/Module.h" "module/Module.cpp" "builtin/BuiltinGarage.h" "builtin/BuiltinGarage.cpp" "vm/primitives/Str.h" "vm/primitives/List.h" "vm/primitives/Dict.h" "vm/primitives/Primitives.h" "vm/primitives/Primitives.inl" "builtin/Math.h" "builtin/Math.cpp" "builtin/InstTest.h"  "builtin/InstTest.cpp" "builtin/Rand.h" "builtin/Rand.cpp")

target_include_directories(ycore
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/contract
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vm
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
