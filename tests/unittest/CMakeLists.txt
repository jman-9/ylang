project ("unittest")

# Add source to this project's executable.
add_executable (unittest "unittest.cpp" "catch2/catch_amalgamated.hpp" "catch2/catch_amalgamated.cpp")

target_link_libraries(unittest
	PRIVATE ycore
)

target_compile_definitions(unittest PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN)

target_include_directories(unittest
	PRIVATE ${CMAKE_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
