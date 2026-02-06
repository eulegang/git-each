cmake_minimum_required(VERSION 3.10)
project(git-each
  LANGUAGES CXX
  VERSION 0.1.1
  DESCRIPTION "broadcast git commands"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(PkgConfig REQUIRED)

file(GLOB EXEC_SRC "src/*.cpp")
add_executable(git-each ${EXEC_SRC})
add_compile_options(-Wall -Wextra -Wpedantic -Werror)
include_directories(src/)

include(CheckSymbolExists)
check_symbol_exists(sendfile "sys/sendfile.h" HAVE_SEND_FILE_LINUX)

if(HAVE_SEND_FILE_LINUX)
  add_definitions(-DHAVE_SEND_FILE_LINUX)
endif()

include(cmake/tests.cmake)

install(TARGETS git-each RUNTIME DESTINATION bin)
