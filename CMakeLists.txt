cmake_minimum_required(VERSION 3.10)

project(fpmas-prey-predator)

find_package(fpmas 1.1 REQUIRED)
add_subdirectory(yaml-cpp)

set(CMAKE_CXX_FLAGS_DEBUG
	"${CMAKE_CXX_FLAGS_DEBUG} -g -O0\
	-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

add_library(fpmas-prey-predator-lib
	src/prey_predator.cpp
	)
add_executable(fpmas-prey-predator
	src/main.cpp
	)

include_directories(include)

target_link_libraries(fpmas-prey-predator-lib fpmas::fpmas yaml-cpp::yaml-cpp)
target_link_libraries(fpmas-prey-predator fpmas-prey-predator-lib)

add_subdirectory(tests)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
