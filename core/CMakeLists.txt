add_library(ylang_core STATIC
	Scanner.cpp
	Scanner.h
 )

target_include_directories(ylang_core
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE${CMAKE_CURRENT_SOURCE_DIR}
)
