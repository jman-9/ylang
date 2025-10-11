# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("devtest")

# Add source to this project's executable.
add_executable (ylang "ylang.cpp" "ylang.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ylang PROPERTY CXX_STANDARD 20)
endif()
