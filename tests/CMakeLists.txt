include(FetchContent)
include(GoogleTest)

set(TESTNAME "tests")
project(${TESTNAME} LANGUAGES CXX)
enable_testing()

file(GLOB TESTED_SOURCES
    ../src/*.h
)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)

set(BUILD_GTEST ON CACHE BOOL "" FORCE)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_test(NAME MyTests COMMAND ${TESTNAME})

add_executable(${TESTNAME} tests.cpp ${TESTED_SOURCES})

target_include_directories(${TESTNAME} PRIVATE ../src)
target_link_libraries(${TESTNAME} gtest gtest_main)

gtest_discover_tests(${TESTNAME})
