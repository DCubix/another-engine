cmake_minimum_required(VERSION 3.11)
project(another)

set(CMAKE_EXPORT_COMPILE_COMMANDS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

file(GLOB SRC
	"src/*.h"
	"src/*.hpp"
	"src/*.cpp"
)

add_definitions(-DSDL_MAIN_HANDLED)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/glad)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/core)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/rendering)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE glad core rendering)

if(CMAKE_DL_LIBS)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})
endif()
