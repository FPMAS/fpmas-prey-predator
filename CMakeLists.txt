cmake_minimum_required(VERSION 3.10)

project(fpmas-prey-predator)

find_package(fpmas 1.0 REQUIRED)
add_subdirectory(yaml-cpp)

set(CMAKE_CXX_FLAGS_DEBUG
	"${CMAKE_CXX_FLAGS_DEBUG} -g -O0\
	-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

add_executable(fpmas-prey-predator
	src/main.cpp)
include_directories(include)

target_link_libraries(fpmas-prey-predator fpmas::fpmas yaml-cpp::yaml-cpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
