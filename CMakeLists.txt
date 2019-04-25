cmake_minimum_required (VERSION 3.8)
project (cpptr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

option(BUILD_TEST "Build the tests of CPPTR" ON)

if (BUILD_TEST)
  add_subdirectory(3rd_party/googletest)
	add_subdirectory(src/test)
endif()