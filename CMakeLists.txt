cmake_minimum_required(VERSION 3.10)
project(git-each
  LANGUAGES CXX
  VERSION 0.1.0
  DESCRIPTION "broadcast git commands"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

file(GLOB EXEC_SRC "src/*.cpp")
add_executable(git-each ${EXEC_SRC})
add_compile_options(-Wall -Wextra -Wpedantic -Werror)
include_directories(src/)


pkg_check_modules(GTEST REQUIRED gtest)
if(NOT GTEST_FOUND)
  message(FATAL_ERROR "failed to find gtest on system")
endif()

enable_testing()

file(GLOB TEST_DIRS "tests/*")

foreach(SUITE ${TEST_DIRS})
  if(${SUITE} MATCHES "/main.cpp$")
    continue()
  endif()
  get_filename_component(TEST_NAME ${SUITE} NAME)

  file(GLOB TEST_SRC "${SUITE}/*.cpp")

  add_executable(tests-${TEST_NAME} ${TEST_SRC} tests/main.cpp $<FILTER:${EXEC_SRC},EXCLUDE,main.cpp>)

  target_link_libraries(tests-${TEST_NAME} PRIVATE ${GTEST_LDFLAGS})
  target_compile_options(tests-${TEST_NAME} PRIVATE ${GTEST_CFLAGS})

  add_test(tests-${TEST_NAME} tests-${TEST_NAME})
endforeach()

