include(FetchContent)
include(GoogleTest)

set(TESTNAME "tests")
set(VULKAN_TESTNAME "vulkan_tests")
project(${TESTNAME} LANGUAGES CXX)
project(${VULKAN_TESTNAME} LANGUAGES CXX)
enable_testing()

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)

set(BUILD_GTEST ON CACHE BOOL "" FORCE)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_test(NAME BasicTests COMMAND ${TESTNAME})
add_test(NAME VulkanTests COMMAND ${VULKAN_TESTNAME})

add_executable(${TESTNAME} tests.cpp)
add_executable(${VULKAN_TESTNAME} vulkan_tests.cpp)

target_include_directories(${TESTNAME} PRIVATE ../src ../src/vulkan)
target_link_libraries(${TESTNAME} ${PROJECT_LIB} gtest gtest_main)
target_include_directories(${VULKAN_TESTNAME} PRIVATE ../src ../src/vulkan ../dependencies)
target_link_libraries(${VULKAN_TESTNAME} ${PROJECT_LIB} gtest gtest_main)
if(MSVC)
  SET_TARGET_PROPERTIES(${VULKAN_TESTNAME} PROPERTIES LINK_FLAGS
    "${LINK_FLAGS} /NODEFAULTLIB:\"msvcrt\"")
  message(STATUS "Link flags ${CMAKE_EXE_LINKER_FLAGS}")
endif()

gtest_discover_tests(${TESTNAME})
gtest_discover_tests(${VULKAN_TESTNAME})
