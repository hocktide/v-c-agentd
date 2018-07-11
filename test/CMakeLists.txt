cmake_minimum_required(VERSION 3.10)
project(agentd-test)

set(CMAKE_CXX_STANDARD 11)

file(GLOB_RECURSE agentd_test_src *.cpp)
file(GLOB_RECURSE agentd_test_headers *.h)

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(
        agentd-test
        ${agentd_test_src}
        ${agentd_test_headers}
)
target_link_libraries(agentd-test ${GTEST_BOTH_LIBRARIES})
