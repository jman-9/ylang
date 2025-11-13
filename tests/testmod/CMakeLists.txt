project ("testmod")

add_library(testmod MODULE testmod.cpp
)

if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

get_target_property(YCORE_INCLUDES ycore INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(testmod
	PRIVATE ${YCORE_INCLUDES}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
