cmake_minimum_required(VERSION 3.10)
project(agentd C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

file(GLOB_RECURSE agentd_src src/*.c)
file(GLOB_RECURSE agentd_headers include/*.h)

add_subdirectory(test)

add_executable(agentd
        ${agentd_src}
        ${agentd_headers}
        )