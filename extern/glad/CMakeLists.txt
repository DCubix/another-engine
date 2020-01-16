cmake_minimum_required(VERSION 3.11)
project(glad)

set(SRC "glad.c")

add_library(glad STATIC ${SRC})
target_include_directories(
	glad
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
)