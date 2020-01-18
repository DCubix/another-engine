cmake_minimum_required(VERSION 3.11)
project(core)

file(GLOB SRC "*.h" "*.hpp" "*.cpp")

find_package(SDL2 CONFIG REQUIRED)
find_package(PhysFS REQUIRED)

add_library(${PROJECT_NAME} STATIC ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2 glad PhysFS)
target_include_directories(
	${PROJECT_NAME}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
)