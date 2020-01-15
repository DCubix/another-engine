cmake_minimum_required(VERSION 3.11)
project(another)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

file(GLOB_RECURSE SRC
	"src/*.h"
	"src/*.hpp"
	"src/*.cpp"
)

find_package(SDL2 CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2)

if(CMAKE_DL_LIBS)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})
endif()
