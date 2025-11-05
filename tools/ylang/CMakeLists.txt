project ("ylang")

# Add source to this project's executable.
add_executable (ylang "main.cpp"  "ylang.h" "ylang.cpp")

target_link_libraries(ylang
	PRIVATE ycore
)

target_include_directories(ylang
	PRIVATE ${CMAKE_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
