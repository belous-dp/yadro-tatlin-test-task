cmake_minimum_required(VERSION 3.22)
project(yadro_tatlin_test_task)


set(CMAKE_CXX_STANDARD 20)

add_executable(${CMAKE_PROJECT_NAME} main.cpp file_tape.cpp tape_utils.cpp tape_algorithm.cpp)
add_executable(tests tests/tests.cpp file_tape.cpp tape_utils.cpp tape_algorithm.cpp)

if (MSVC)
    add_compile_options(/W4)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-stdlib=libc++)
endif ()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_options(-D_GLIBCXX_DEBUG)
endif ()

option(USE_SANITIZERS "Enable to build with undefined, leak and address sanitizers" OFF)
if (USE_SANITIZERS AND NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_compile_options(-fsanitize=address,undefined,leak -fno-sanitize-recover=all)
    add_link_options(-fsanitize=address,undefined,leak)
endif ()


find_package(args REQUIRED)
find_package(GTest REQUIRED)

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC taywee::args)
target_link_libraries(tests PUBLIC GTest::gtest GTest::gtest_main)
target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR})
