add_library(ycore STATIC
  "contract/Token.h" "contract/Token.cpp" "contract/Symbol.h" "contract/InstructionSet.h" "contract/Instruction.h" "contract/Instruction.inl" "contract/Bytecode.h" "contract/Program.h" "contract/ycontract.h" "contract/ycontract.inl"
  "compiler/TreeNode.h" "contract/Error.h" "contract/Error.cpp" "compiler/Scanner.h" "compiler/Scanner.cpp" "compiler/StringInterpolator.h" "compiler/StringInterpolator.cpp" "compiler/Parser.h" "compiler/Parser.cpp" "compiler/SemanticAnalyzer.h" "compiler/SemanticAnalyzer.cpp"  "compiler/BytecodeBuilder.h" "compiler/BytecodeBuilder.cpp" "compiler/Compiler.h" "compiler/Compiler.cpp"
  "vm/Machine.cpp" "vm/yvm.h"  "vm/Machine.h" "vm/primitives/Str.h" "vm/Variable.h" "vm/Variable.cpp" "vm/primitives/List.h" "vm/primitives/Dict.h" "vm/primitives/Primitives.h" "vm/primitives/Primitives.inl"
  "module/Module.h" "module/Module.cpp" "module/ModuleManager.h" "module/ModuleManager.cpp"
  "builtin/BuiltinGarage.h" "builtin/BuiltinGarage.cpp"  "builtin/Math.h" "builtin/Math.cpp" "builtin/InstTest.h"  "builtin/InstTest.cpp" "builtin/Rand.h" "builtin/Rand.cpp" "builtin/Sys.h"  "builtin/Sys.cpp""builtin/File.cpp" "builtin/File.h" "builtin/Json.cpp" "builtin/Json.h"
  "util/Args.h" "util/Args.cpp" "util/StrUtil.h" "util/StrUtil.inl"  "compiler/NamespaceUtil.h" "compiler/NamespaceUtil.cpp" "ylangDefs.h" "compiler/ScopeManager.h" "compiler/ScopeManager.cpp")

target_include_directories(ycore
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/contract
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/compiler
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vm
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/util
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
