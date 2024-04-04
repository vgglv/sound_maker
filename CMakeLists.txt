cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(sound_maker)

# APPLICATION
file(GLOB_RECURSE SOURCES 
	"src/*.cpp"
	"src/*.h"
)

set(EXEC_VARIABLE)
if (WIN32)
	set(EXEC_VARIABLE WIN32)
endif(WIN32)
add_executable(${PROJECT_NAME} ${EXEC_VARIABLE} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)
target_include_directories(${PROJECT_NAME} PUBLIC ${CWD}/src)

set_target_properties(${PROJECT_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}
)

target_link_libraries(sound_maker winmm.lib)
