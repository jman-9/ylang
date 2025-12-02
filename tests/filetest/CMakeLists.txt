project ("filetest")

# Add source to this project's executable.
add_executable (filetest "filetest.cpp" "../../tools/ylang/ylang.h" "../../tools/ylang/ylang.cpp")

target_link_libraries(filetest
	PRIVATE ycore
)

target_include_directories(filetest
	PRIVATE ${CMAKE_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
