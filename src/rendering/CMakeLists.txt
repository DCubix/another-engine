cmake_minimum_required(VERSION 3.11)
project(rendering)

file(GLOB SRC "*.h" "*.cpp")

add_library(${PROJECT_NAME} STATIC ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE glad core)
target_include_directories(
	${PROJECT_NAME}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
)