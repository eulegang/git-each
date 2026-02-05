pkg_check_modules(GTEST gtest)

if(GTEST_FOUND)
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
endif()
