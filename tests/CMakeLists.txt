add_subdirectory(googletest)

include_directories(../include include)

add_executable(fpmas-prey-predator-test
	src/main.cpp
	src/classic_pp.cpp
	src/mock_prey_predator.cpp
	)
target_link_libraries(
	fpmas-prey-predator-test fpmas-prey-predator-lib fpmas::fpmas
	gtest_main gmock_main)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
