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
	src/classic_pp.cpp
	src/constrained_pp.cpp
	src/output.cpp
	src/config.cpp
	src/json_setup.cpp
	)
add_executable(fpmas-prey-predator
	src/main.cpp
	)

include_directories(include)

target_link_libraries(fpmas-prey-predator-lib fpmas::fpmas yaml-cpp::yaml-cpp)
target_link_libraries(fpmas-prey-predator fpmas-prey-predator-lib)

add_subdirectory(tests)

find_package(Doxygen)
if(DOXYGEN_FOUND)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs)
	set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs)
	set(DOXYGEN_MACRO_EXPANSION "YES")
	set(DOXYGEN_HTML_OUTPUT ".")
	#set(DOXYGEN_CLANG_ASSISTED_PARSING "YES")
	#set(DOXYGEN_CLANG_DATABASE_PATH "build/compile_commands.json")
	set(DOXYGEN_SORT_MEMBER_DOCS "NO")
	set(DOXYGEN_PREDEFINED "DOXYGEN_BUILD")
	set(DOXYGEN_HAVE_DOT "YES")
	#set(DOXYGEN_TEMPLATE_RELATIONS "YES")
	set(DOXYGEN_VERBATIM_VARS DOXYGEN_ALIASES)
	set(DOXYGEN_HIDE_FRIEND_COMPOUNDS "YES")
	doxygen_add_docs(doc
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/src
		)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
