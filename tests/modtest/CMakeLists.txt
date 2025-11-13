project ("modtest")

# Add source to this project's executable.
add_executable (modtest "modtest.cpp")

target_link_libraries(modtest
	PRIVATE ycore
)

target_include_directories(modtest
	PRIVATE ${CMAKE_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
