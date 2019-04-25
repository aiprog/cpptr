cmake_minimum_required (VERSION 3.8)
project(cpptr_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/src/cpptr)
include_directories(${CMAKE_SOURCE_DIR}/3rd_party/googletest/googletest/include)

add_executable(cpptr_test type_desc.cpp cpptr.cpp)

if (MSVC)
 target_compile_options(cpptr_test PRIVATE /W3)
else()
 target_compile_options(cpptr_test PRIVATE -Wall)
endif()

target_link_libraries(cpptr_test gtest_main)