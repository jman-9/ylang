project ("devtest")

# Add source to this project's executable.
add_executable (devtest "devtest.cpp")

set_property(TARGET devtest PROPERTY CXX_STANDARD 20)

target_link_libraries(devtest
	PRIVATE ycore
)

target_include_directories(devtest
	PRIVATE ${CMAKE_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
