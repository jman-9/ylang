project ("devtest")

# Add source to this project's executable.
add_executable (devtest "devtest.cpp")

target_link_libraries(devtest
	PRIVATE ycore
)

target_include_directories(devtest
	PRIVATE ${CMAKE_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
